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

                    DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
                    Cursor cursor = downloadManager.query(new DownloadManager.Query().setFilterById(id));

                    int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                    int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

                    int status = -1;
                    int reason = -1;

                    if (cursor.moveToFirst())
                    {
                        status = cursor.getInt(statusIndex);
                        reason = cursor.getInt(reasonIndex);
                    }

                    DownloadComplete(id, status, reason);
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

                    int idIndex = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
                    int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
                    int downloadBytesIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
                    int totalBytesIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
                    int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                    int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

                    if (!cursor.moveToFirst())
                        return;

                    do
                    {
                        String localUri = cursor.getString(localUriIndex);
                        if (!uris.contains(Uri.parse(localUri)))
                            continue;

                        long id = cursor.getInt(idIndex);

                        Log.d("DownloadService::downloadProgressObserver::onChange", String.format("id=%d localUri=%s", id, localUri));

                        long cur = cursor.getInt(downloadBytesIndex);
                        long total = cursor.getInt(totalBytesIndex);
                        int status = cursor.getInt(statusIndex);
                        int reason = cursor.getInt(reasonIndex);

                        Log.d("DownloadService::downloadProgressObserver::onChange", String.format("Status %d, Reason %d", status, reason));
                        Log.d("DownloadService::downloadProgressObserver::onChange", String.format("Downloaded %d, Total %d %f%%",
                                cur, total, total > 0 ? cur * 100.0 / total : 0));

                        if (status == DownloadManager.STATUS_FAILED)
                            DownloadComplete(id, status, reason);

                        // TODO: Send download progress event
                    } while(cursor.moveToNext());
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

    public static void SyncDownloads(Context context)
    {
        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        Cursor cursor = downloadManager.query(new DownloadManager.Query());

        Log.d("DownloadService::SyncDownloads", String.format("%d records", cursor.getCount()));

        int idIndex = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
        int titleIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TITLE);
        int descriptionIndex = cursor.getColumnIndex(DownloadManager.COLUMN_DESCRIPTION);
        int uriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_URI);
        int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        int downloadedIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
        int sizeIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
        int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
        int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);
        int timestampIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LAST_MODIFIED_TIMESTAMP);
        int mediaTypeIndex = cursor.getColumnIndex(DownloadManager.COLUMN_MEDIA_TYPE);
        int mediaProviderUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_MEDIAPROVIDER_URI);

        Log.d("", "ID TITLE DESCRIPTION URI LOCAL_URI DOWNLOADED SIZE STATUS REASON TIMESTAMP TYPE MEDIAPROVIDER_URI");

        if (!cursor.moveToFirst())
            return;

        do
        {
            long id = cursor.getInt(idIndex);
            String title = cursor.getString(titleIndex);
            String description = cursor.getString(descriptionIndex);
            String uri = cursor.getString(uriIndex);
            String localUri = cursor.getString(localUriIndex);
            long downloaded = cursor.getLong(downloadedIndex);
            long size = cursor.getLong(sizeIndex);
            int status = cursor.getInt(statusIndex);
            int reason = cursor.getInt(reasonIndex);
            long timestamp = cursor.getLong(timestampIndex);
            String mediaType = cursor.getString(mediaTypeIndex);
            String mediaProviderUri = cursor.getString(mediaProviderUriIndex);

            Log.d("", String.format("%d %s %s %s %s %d %d %d %d %d %s %s",
                id, title, description, uri, localUri, downloaded, size, status, reason, timestamp, mediaType, mediaProviderUri));

            // TODO: Send download progress event

            if (status != DownloadManager.STATUS_FAILED && status != DownloadManager.STATUS_SUCCESSFUL)
                RegisterObserver(context, localUri);
            else
                DownloadComplete(id, status, reason);

            RegisterReceiver(context);
        } while(cursor.moveToNext());
    }

    private static native void DownloadComplete(long id, int status, int reason);

    private static BroadcastReceiver downloadEventsReceiver = null;
    private static ContentObserver downloadProgressObserver = null;
}
