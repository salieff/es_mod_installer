package org.salieff;

import android.net.Uri;
import android.os.Parcelable;
import android.provider.DocumentsContract;
import android.content.Intent;
import android.content.Context;
import android.content.UriPermission;
import android.os.ParcelFileDescriptor;
import android.database.Cursor;
import android.provider.OpenableColumns;
import android.util.Log;
import java.net.URLEncoder;


public class SafAdapter
{
    Uri m_rootUri = null;
    String m_safRoot = null;
    String m_lastErrorString = null;

    public SafAdapter(String root)
    {
        m_safRoot = root;
    }

    private String getStackTraceAsString(Throwable throwable)
    {
        return "m_rootUri = " + (m_rootUri == null ? "null" : m_rootUri.toString()) + "\n" +
               "m_safRoot = " + (m_safRoot == null ? "null" : m_safRoot) + "\n" +
               Log.getStackTraceString(throwable);
    }

    public String lastErrorString()
    {
        return m_lastErrorString;
    }

    public boolean rootUriPermissionGranted(Context context)
    {
        m_lastErrorString = null;

        try 
        {
            for (UriPermission p : context.getContentResolver().getPersistedUriPermissions())
            {
                if (p.getUri().toString().equals("content://com.android.externalstorage.documents/tree/primary%3A" + URLEncoder.encode(m_safRoot, "utf-8")) && p.isReadPermission() && p.isWritePermission())
                {
                    m_rootUri = p.getUri();
                    return true;
                }
            }
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }

        return false;
    }

    public Intent intentForRootUriPermissionRequest()
    {
        Intent intent = new Intent("android.intent.action.OPEN_DOCUMENT_TREE");

        Uri intentRootUri = DocumentsContract.buildDocumentUri("com.android.externalstorage.documents", "primary:" + m_safRoot);
        intent.putExtra("android.provider.extra.INITIAL_URI", (Parcelable)intentRootUri);

        /*
        intent.putExtra("android.content.extra.SHOW_ADVANCED", true);
        intent.putExtra("android.content.extra.FANCY", true);
        intent.putExtra("android.content.extra.SHOW_FILESIZE", true);
        */

        return intent;
    }

    public void takeRootUriPermission(Context context, Intent intent)
    {
        m_lastErrorString = null;

        try
        {
            Uri intentRootUri = intent.getData();
            if (intentRootUri == null || !"com.android.externalstorage.documents".equals(intentRootUri.getAuthority()))
                return;

            String docId;
            try
            {
                docId = DocumentsContract.getTreeDocumentId(intentRootUri);   // уже декодирован
            }
            catch (IllegalArgumentException e)
            {
                return;
            }

            if (!docId.equals("primary:" + m_safRoot))
                return;

            context.getContentResolver().takePersistableUriPermission(intentRootUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_READ_URI_PERMISSION);
            m_rootUri = intentRootUri;
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }
    }

    public boolean createFolder(Context context, String rootFolderName, String subFolderName)
    {
        m_lastErrorString = null;

        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            Uri subFolderUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, DocumentsContract.Document.MIME_TYPE_DIR, subFolderName);

            return true;
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }

        return false;
    }

    public int openFile(Context context, String rootFolderName, String fileName, String mode)
    {
        m_lastErrorString = null;
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName + "/" + fileName);
            ParcelFileDescriptor fileDescriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDescriptor.detachFd();
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }

        return -1;
    }

    public int createFile(Context context, String rootFolderName, String fileName, String mode)
    {
        m_lastErrorString = null;
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, "application/octet-stream", fileName);
            ParcelFileDescriptor fileDescriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDescriptor.detachFd();
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }

        return -1;
    }

    public boolean deleteFile(Context context, String fileName)
    {
        m_lastErrorString = null;

        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + fileName);
            return DocumentsContract.deleteDocument(context.getContentResolver(), fileUri);
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }

        return false;
    }

    public int folderSize(Context context, String folderName)
    {
        int retValue = -1;
        Cursor cursor = null;
        m_lastErrorString = null;

        try
        {
            Uri folderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + folderName);
            cursor = context.getContentResolver().query(folderUri, new String[]{DocumentsContract.Document.COLUMN_DOCUMENT_ID}, null, null, null);

            if (cursor == null)
                retValue = -1;
            else
                retValue = cursor.getCount();
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
            retValue = -1;
        }
        finally
        {
            if (cursor != null)
                cursor.close();
        }

        return retValue;
    }

    public long fileSize(Context context, String fileName)
    {
        long retValue = -1;
        Cursor cursor = null;
        m_lastErrorString = null;

        try
        {
            Uri fileUri = DocumentsContract.buildDocumentUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + fileName);

            String [] requestedColumns = {OpenableColumns.SIZE};
            cursor = context.getContentResolver().query(fileUri, requestedColumns, null, null, null);
            if (cursor == null)
                return -1;

            int sizeIndex = cursor.getColumnIndex(OpenableColumns.SIZE);
            if (sizeIndex >= 0 && cursor.moveToFirst())
                retValue = cursor.getLong(sizeIndex);
        }
        catch (Exception e)
        {
            m_lastErrorString = getStackTraceAsString(e);
            e.printStackTrace();
        }
        finally
        {
            if (cursor != null)
                cursor.close();
        }

        return retValue;
    }
}
