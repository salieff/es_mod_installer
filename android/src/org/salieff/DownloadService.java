package org.salieff;

import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.database.Cursor;
import android.os.Environment;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import java.util.*;
import android.util.Log;


// TODO: android ContentResolver ContentObserver file onchange example
//       DownloadManager.COLUMN_LOCAL_URI
//       context.getContentResolver().registerContentObserver()


public class DownloadService
{
    public static void RegisterReceiver(Context context)
    {
        if (downloadEventsReceiver == null)
        {
            downloadEventsReceiver = new BroadcastReceiver()
            {
                @Override
                public void onReceive(Context context, Intent intent)
                {
                    long id = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1);
                    DownloadComplete(id);
                }
            };

            context.registerReceiver(downloadEventsReceiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE), Context.RECEIVER_EXPORTED);
        }
    }

    public static void RegisterObserver(Context context, String localUri)
    {
        if (downloadProgressObserver == null)
        {
            downloadProgressObserver = new ContentObserver(new Handler(Looper.getMainLooper()))
            {
                @Override
                public void onChange(boolean selfChange)
                {
                    onChange(selfChange, null);
                }

                @Override
                public void onChange(boolean selfChange, Uri uri)
                {
                    onChange(selfChange, uri, 0);
                }

                @Override
                public void onChange(boolean selfChange, Uri uri, int flags)
                {
                    onChange(selfChange, Collections.singletonList(uri), flags);
                }

                @Override
                public void onChange(boolean selfChange, Collection<Uri> uris, int flags)
                {
                    DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
                    Cursor cursor = downloadManager.query(new DownloadManager.Query());
                    int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
                    int downloadBytesIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
                    int totalBytesIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);

                    for (Uri u : uris)
                    {
                        Log.d("DownloadService::downloadProgressObserver::onChange", u.toString());

                        if (!cursor.moveToFirst())
                            continue;

                        do
                        {
                            if (!cursor.getString(localUriIndex).equals(u.toString()))
                                continue;

                            int cur = cursor.getInt(downloadBytesIndex);
                            int total = cursor.getInt(totalBytesIndex);

                            if (total <= 0)
                                break;

                            Log.d("DownloadService::downloadProgressObserver::onChange", String.format("Downloaded %d, Total %d %f%%", cur, total, cur * 100.0 / total));
                        } while(cursor.moveToNext());
                    }
                }
            };
        }

        context.getContentResolver().registerContentObserver(Uri.parse(localUri), false, downloadProgressObserver);
    }

    public static long StartDownload(Context context, String url)
    {
        RegisterReceiver(context);

        String fileName = url.substring(url.lastIndexOf('/') + 1);

        DownloadManager.Request request = new DownloadManager.Request(Uri.parse(url))
        //  .setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, fileName)
            .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE)
            .setTitle(fileName)
            .setDescription(url);

        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        long downloadId = downloadManager.enqueue(request);

        Cursor cursor = downloadManager.query(new DownloadManager.Query().setFilterById(downloadId));
        int index = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        cursor.moveToFirst();
        String localUri = cursor.getString(index);
        Log.d("DownloadService::StartDownload", localUri);

        RegisterObserver(context, localUri);

        return downloadId;
    }

    private static native void DownloadComplete(long id);

    private static BroadcastReceiver downloadEventsReceiver = null;
    private static ContentObserver downloadProgressObserver = null;
}
