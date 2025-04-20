package org.salieff;

import android.content.Context;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.database.Cursor;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import java.util.Collection;
import java.util.Collections;


public class ADMController
{
    private long m_cppThisPointer = -1;
    private Context m_context = null;
    private DownloadManager m_downloadManager = null;
    private long m_downloadId = -1;
    private static BroadcastReceiver m_downloadEventsReceiver = null;
    private static ContentObserver m_downloadProgressObserver = null;


    public ADMController(Context context, long cppThisPointer)
    {
        m_context = context;
        m_cppThisPointer = cppThisPointer;
        m_downloadManager = (DownloadManager) m_context.getSystemService(Context.DOWNLOAD_SERVICE);

        CreateObserver();
        CreateAndRegisterReceiver();
    }

    public boolean sync(String serverUrl, String filePath)
    {
        m_downloadId = -1;
        m_context.getContentResolver().unregisterContentObserver(m_downloadProgressObserver);

        Cursor cursor = m_downloadManager.query(new DownloadManager.Query());
        if (!cursor.moveToFirst())
            return false;

        int idIndex = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
        int uriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_URI);
        int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        int downloadedIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
        int sizeIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
        int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
        int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

        String syncUrl = Uri.withAppendedPath(Uri.parse(serverUrl), filePath).toString();

        do
        {
            String uri = cursor.getString(uriIndex);
            if (!uri.equals(syncUrl))
                continue;

            m_downloadId = cursor.getInt(idIndex);

            String localUri = cursor.getString(localUriIndex);
            long downloaded = cursor.getLong(downloadedIndex);
            long size = cursor.getLong(sizeIndex);
            int status = cursor.getInt(statusIndex);
            int reason = cursor.getInt(reasonIndex);

            DownloadProgress(m_cppThisPointer, m_downloadId, downloaded, size);

            if (status != DownloadManager.STATUS_FAILED && status != DownloadManager.STATUS_SUCCESSFUL)
                m_context.getContentResolver().registerContentObserver(Uri.parse(localUri), false, m_downloadProgressObserver);
            else
                DownloadComplete(m_cppThisPointer, m_downloadId, status, reason);

            return true;
        } while(cursor.moveToNext());

        return false;
    }

    public boolean download(String serverUrl, String filePath, String title, String description)
    {
        if (sync(serverUrl, filePath))
            return true;

        DownloadManager.Request request = new DownloadManager.Request(Uri.withAppendedPath(Uri.parse(serverUrl), filePath))
            .setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE)
            .setTitle(title)
            .setDescription(description);

        m_downloadId = m_downloadManager.enqueue(request);
        if (m_downloadId == -1)
            return false;

        Cursor cursor = m_downloadManager.query(new DownloadManager.Query().setFilterById(m_downloadId));
        if (!cursor.moveToFirst())
            return false;

        int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        String localUri = cursor.getString(localUriIndex);

        m_context.getContentResolver().registerContentObserver(Uri.parse(localUri), false, m_downloadProgressObserver);
        return true;
    }

    public int open()
    {
        return openHelper(m_downloadManager, m_downloadId);
    }

    public void remove()
    {
        m_downloadManager.remove(m_downloadId);
        m_downloadId = -1;
    }

    public static int open(Context context, long id)
    {
        return openHelper((DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE), id);
    }

    private static int openHelper(DownloadManager downloadManager, long id)
    {
        try
        {
            ParcelFileDescriptor fileDiscriptor = downloadManager.openDownloadedFile(id);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public static void remove(Context context, long id)
    {
        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        downloadManager.remove(id);
    }

    private static native void DownloadComplete(long cppThisPpointer, long id, int status, int reason);
    private static native void DownloadProgress(long cppThisPpointer, long id, long downloaded, long totalSize);

    private void CreateAndRegisterReceiver()
    {
        if (m_downloadEventsReceiver != null)
            return;

        m_downloadEventsReceiver = new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context, Intent intent)
            {
                if (intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1) != m_downloadId)
                    return;

                Cursor cursor = m_downloadManager.query(new DownloadManager.Query().setFilterById(m_downloadId));

                if (!cursor.moveToFirst())
                    return;

                int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

                int status = cursor.getInt(statusIndex);
                int reason = cursor.getInt(reasonIndex);

                DownloadComplete(m_cppThisPointer, m_downloadId, status, reason);
                m_context.getContentResolver().unregisterContentObserver(m_downloadProgressObserver);
            }
        };

        m_context.registerReceiver(m_downloadEventsReceiver, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE), Context.RECEIVER_EXPORTED);
    }

    private void CreateObserver()
    {
        if (m_downloadProgressObserver != null)
            return;

        m_downloadProgressObserver = new ContentObserver(new Handler(Looper.getMainLooper()))
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
                Cursor cursor = m_downloadManager.query(new DownloadManager.Query());

                if (!cursor.moveToFirst())
                    return;

                int idIndex = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
                int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
                int downloadedIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
                int sizeIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
                int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
                int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

                do
                {
                    String localUri = cursor.getString(localUriIndex);
                    if (!uris.contains(Uri.parse(localUri)))
                        continue;

                    long id = cursor.getInt(idIndex);
                    if (id != m_downloadId)
                        continue;

                    long downloaded = cursor.getInt(downloadedIndex);
                    long size = cursor.getInt(sizeIndex);
                    int status = cursor.getInt(statusIndex);
                    int reason = cursor.getInt(reasonIndex);

                    DownloadProgress(m_cppThisPointer, m_downloadId, downloaded, size);

                    if (status == DownloadManager.STATUS_FAILED)
                    {
                        DownloadComplete(m_cppThisPointer, m_downloadId, status, reason);
                        m_context.getContentResolver().unregisterContentObserver(this);
                    }

                    break;
                } while(cursor.moveToNext());
            }
        };
    }
}
