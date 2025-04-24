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


class ADMReceiver extends BroadcastReceiver
{
    private Context m_context;
    private DownloadManager m_downloadManager = null;
    private long m_cppThisPointer = -1;
    private long m_admId = -1;

    public ADMObserver m_observer = null;

    public ADMReceiver(Context context, DownloadManager downloadManager, long cppThisPointer)
    {
        m_context = context;
        m_downloadManager = downloadManager;
        m_cppThisPointer = cppThisPointer;
    }

    public void register(long id)
    {
        m_admId = id;
        m_context.registerReceiver(this, new IntentFilter(DownloadManager.ACTION_DOWNLOAD_COMPLETE), Context.RECEIVER_EXPORTED);
    }

    public void unregister()
    {
        try
        {
            m_context.unregisterReceiver(this);
        }
        catch (Exception e)
        {
            // Ignore java.lang.IllegalArgumentException: Receiver not registered: org.salieff.ADMReceiver@dc24084
            // e.printStackTrace();
        }

        m_admId = -1;
    }

    @Override
    public void onReceive(Context context, Intent intent)
    {
        long id = intent.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, -1);
        if (id != m_admId)
            return;

        Cursor cursor = m_downloadManager.query(new DownloadManager.Query().setFilterById(id));

        if (!cursor.moveToFirst())
        {
            cursor.close();
            return;
        }

        int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
        int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

        int status = cursor.getInt(statusIndex);
        int reason = cursor.getInt(reasonIndex);

        cursor.close();

        m_observer.unregister();
        unregister();

        ADMController.DownloadComplete(m_cppThisPointer, id, status, reason);
    }
}


class ADMObserver extends ContentObserver
{
    private Context m_context;
    private DownloadManager m_downloadManager = null;
    private long m_cppThisPointer = -1;
    private long m_admId = -1;

    public ADMReceiver m_receiver = null;

    public ADMObserver(Context context, DownloadManager downloadManager, long cppThisPointer)
    {
        super(new Handler(Looper.getMainLooper()));

        m_context = context;
        m_downloadManager = downloadManager;
        m_cppThisPointer = cppThisPointer;
    }

    public void register(long id, String localUri)
    {
        m_admId = id;
        m_context.getContentResolver().registerContentObserver(Uri.parse(localUri), false, this);
    }

    public void unregister()
    {
        m_context.getContentResolver().unregisterContentObserver(this);
        m_admId = -1;
    }

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
        {
            cursor.close();
            return;
        }

        int idIndex = cursor.getColumnIndex(DownloadManager.COLUMN_ID);
        int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        int downloadedIndex = cursor.getColumnIndex(DownloadManager.COLUMN_BYTES_DOWNLOADED_SO_FAR);
        int sizeIndex = cursor.getColumnIndex(DownloadManager.COLUMN_TOTAL_SIZE_BYTES);
        int statusIndex = cursor.getColumnIndex(DownloadManager.COLUMN_STATUS);
        int reasonIndex = cursor.getColumnIndex(DownloadManager.COLUMN_REASON);

        do
        {
            long id = cursor.getInt(idIndex);
            if (id != m_admId)
                continue;

            String localUri = cursor.getString(localUriIndex);
            if (!uris.contains(Uri.parse(localUri)))
                continue;

            long downloaded = cursor.getInt(downloadedIndex);
            long size = cursor.getInt(sizeIndex);
            int status = cursor.getInt(statusIndex);
            int reason = cursor.getInt(reasonIndex);

            ADMController.DownloadProgress(m_cppThisPointer, id, downloaded, size);

            // BroadcastReceiver doesn't get ACTION_DOWNLOAD_COMPLETE with STATUS_FAILED for some reason
            // So we have to send DownloadComplete(FAILED) artificially, here
            if (status == DownloadManager.STATUS_FAILED)
            {
                unregister();
                m_receiver.unregister();
                ADMController.DownloadComplete(m_cppThisPointer, id, status, reason);
            }

            break;
        } while(cursor.moveToNext());

        cursor.close();
    }
}


public class ADMController
{
    private long m_cppThisPointer = -1;
    private Context m_context = null;
    private DownloadManager m_downloadManager = null;
    private long m_downloadId = -1;
    private ADMReceiver m_downloadEventsReceiver = null;
    private ADMObserver m_downloadProgressObserver = null;


    public ADMController(Context context, long cppThisPointer)
    {
        m_context = context;
        m_cppThisPointer = cppThisPointer;
        m_downloadManager = (DownloadManager) m_context.getSystemService(Context.DOWNLOAD_SERVICE);

        m_downloadEventsReceiver = new ADMReceiver(m_context, m_downloadManager, m_cppThisPointer);
        m_downloadProgressObserver = new ADMObserver(m_context, m_downloadManager, m_cppThisPointer);

        m_downloadEventsReceiver.m_observer = m_downloadProgressObserver;
        m_downloadProgressObserver.m_receiver = m_downloadEventsReceiver;
    }

    public boolean sync(String serverUrl, String filePath)
    {
        m_downloadId = -1;
        m_downloadProgressObserver.unregister();
        m_downloadEventsReceiver.unregister();

        Cursor cursor = m_downloadManager.query(new DownloadManager.Query());
        if (!cursor.moveToFirst())
        {
            cursor.close();
            return false;
        }

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
            if (m_downloadId < 0)
                continue;

            String localUri = cursor.getString(localUriIndex);
            long downloaded = cursor.getLong(downloadedIndex);
            long size = cursor.getLong(sizeIndex);
            int status = cursor.getInt(statusIndex);
            int reason = cursor.getInt(reasonIndex);

            DownloadProgress(m_cppThisPointer, m_downloadId, downloaded, size);

            if (status != DownloadManager.STATUS_FAILED && status != DownloadManager.STATUS_SUCCESSFUL)
            {
                m_downloadProgressObserver.register(m_downloadId, localUri);
                m_downloadEventsReceiver.register(m_downloadId);
            }
            else
            {
                DownloadComplete(m_cppThisPointer, m_downloadId, status, reason);
            }

            break;
        } while(cursor.moveToNext());

        cursor.close();
        return m_downloadId >= 0;
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
        {
            cursor.close();
            return false;
        }

        int localUriIndex = cursor.getColumnIndex(DownloadManager.COLUMN_LOCAL_URI);
        String localUri = cursor.getString(localUriIndex);

        cursor.close();

        m_downloadProgressObserver.register(m_downloadId, localUri);
        m_downloadEventsReceiver.register(m_downloadId);

        return true;
    }

    public int open()
    {
        return openHelper(m_downloadManager, m_downloadId);
    }

    public void remove()
    {
        m_downloadProgressObserver.unregister();
        m_downloadEventsReceiver.unregister();
        m_downloadManager.remove(m_downloadId);
        m_downloadId = -1;
    }

    public static int Open(Context context, long id)
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

    public static void Remove(Context context, long id)
    {
        DownloadManager downloadManager = (DownloadManager) context.getSystemService(Context.DOWNLOAD_SERVICE);
        downloadManager.remove(id);
    }

    public static native void DownloadComplete(long cppThisPointer, long id, int status, int reason);
    public static native void DownloadProgress(long cppThisPointer, long id, long downloaded, long totalSize);
}
