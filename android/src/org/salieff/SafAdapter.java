package org.salieff;

import android.net.Uri;
import android.os.Parcelable;
import android.provider.DocumentsContract;
import android.content.Intent;
import android.app.Activity;
import android.content.Context;
import android.content.UriPermission;
import android.os.ParcelFileDescriptor;
import android.database.Cursor;
import android.provider.OpenableColumns;
import java.net.URLEncoder;


public class SafAdapter
{
    Uri m_rootUri = null;
    String m_safRoot = null;

    public SafAdapter(String root)
    {
        m_safRoot = root;
    }

    public boolean rootUriPermissionGranted(Context context)
    {
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
        try
        {
            Uri intentRootUri = intent.getData();
            if (!intentRootUri.toString().equals("content://com.android.externalstorage.documents/tree/primary%3A" + URLEncoder.encode(m_safRoot, "utf-8")))
                return;

            context.getContentResolver().takePersistableUriPermission(intentRootUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_READ_URI_PERMISSION);
            m_rootUri = intentRootUri;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    public boolean createFolder(Context context, String rootFolderName, String subFolderName)
    {
        try
        {
            if (folderSize(context, rootFolderName + "/" + subFolderName) >= 0)
                return true;

            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            Uri subFolderUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, DocumentsContract.Document.MIME_TYPE_DIR, subFolderName);

            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public int openFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName + "/" + fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public int createFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + rootFolderName);
            int fd = openFile(context, rootFolderName, fileName, mode);
            if (fd >= 0)
                return fd;

            Uri fileUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, "application/octet-stream", fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public boolean deleteFile(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(m_rootUri, DocumentsContract.getTreeDocumentId(m_rootUri) + fileName);
            return DocumentsContract.deleteDocument(context.getContentResolver(), fileUri);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public int folderSize(Context context, String folderName)
    {
        int retValue = -1;
        Cursor cursor = null;

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
            // e.printStackTrace();
        }
        finally
        {
            if (cursor != null)
                cursor.close();
        }

        return retValue;
    }
}
