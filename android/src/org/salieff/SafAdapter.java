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
import android.util.Log;

public class SafAdapter
{
    public static final int RequestCode = 513375;
    public static Uri RootUri = null;

    public static boolean rootUriPermissionGranted(Context context)
    {
        for (UriPermission p : context.getContentResolver().getPersistedUriPermissions())
        {
            if (p.getUri().toString().equals("content://com.android.externalstorage.documents/tree/primary%3A") && p.isReadPermission() && p.isWritePermission())
            {
                RootUri = p.getUri();
                return true;
            }
        }

        return false;
    }

    public static Intent intentForRootUriPermissionRequest()
    {
        Uri intentRootUri = DocumentsContract.buildDocumentUri("com.android.externalstorage.documents", "primary:");
        return (new Intent("android.intent.action.OPEN_DOCUMENT_TREE")).putExtra("android.provider.extra.INITIAL_URI", (Parcelable)intentRootUri);
    }

    public static void takeRootUriPermission(Context context, Intent intent)
    {
        Uri intentRootUri = intent.getData();
        if (!intentRootUri.toString().equals("content://com.android.externalstorage.documents/tree/primary%3A"))
            return;

        context.getContentResolver().takePersistableUriPermission(intentRootUri, Intent.FLAG_GRANT_WRITE_URI_PERMISSION | Intent.FLAG_GRANT_READ_URI_PERMISSION);
        RootUri = intentRootUri;
    }

    public static boolean createFolder(Context context, String rootFolderName, String subFolderName)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            Uri subFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName + "/" + subFolderName);
            try
            {
                DocumentsContract.Path filePath = DocumentsContract.findDocumentPath(context.getContentResolver(), subFolderUri);
                Log.d("Folder already exists: ", subFolderUri.toString());
            }
            catch (Exception e)
            {
                // e.printStackTrace();
                subFolderUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, DocumentsContract.Document.MIME_TYPE_DIR, subFolderName);
                Log.d("Folder created: ", subFolderUri.toString());
            }

            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public static int openFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName + "/" + fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public static int createFile(Context context, String rootFolderName, String fileName, String mode)
    {
        try
        {
            Uri rootFolderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName);
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + rootFolderName + "/" + fileName);
            try
            {
                DocumentsContract.Path filePath = DocumentsContract.findDocumentPath(context.getContentResolver(), fileUri);
                Log.d("File already exists: ", fileUri.toString());
            }
            catch (Exception e)
            {
                // e.printStackTrace();
                fileUri = DocumentsContract.createDocument(context.getContentResolver(), rootFolderUri, "application/octet-stream", fileName);
                Log.d("File created: ", fileUri.toString());
            }

            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, mode);
            return fileDiscriptor.detachFd();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }

    public static boolean fileExists(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + fileName);
            DocumentsContract.Path filePath = DocumentsContract.findDocumentPath(context.getContentResolver(), fileUri);

            return true;
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public static boolean deleteFile(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + fileName);
            return DocumentsContract.deleteDocument(context.getContentResolver(), fileUri);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return false;
    }

    public static boolean folderEmpty(Context context, String folderName)
    {
        boolean retValue = false;
        Cursor cursor = null;

        try
        {
            Uri folderUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + folderName);
            cursor = context.getContentResolver().query(folderUri, new String[]{DocumentsContract.Document.COLUMN_DOCUMENT_ID}, null, null, null);

            retValue = (cursor != null && cursor.getCount() == 0);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            retValue = false;
        }
        finally
        {
            if (cursor != null)
                cursor.close();
        }

        return retValue;
    }

    public static long fileSize(Context context, String fileName)
    {
        try
        {
            Uri fileUri = DocumentsContract.buildChildDocumentsUriUsingTree(RootUri, DocumentsContract.getTreeDocumentId(RootUri) + fileName);
            ParcelFileDescriptor fileDiscriptor = context.getContentResolver().openFileDescriptor(fileUri, "r");
            return fileDiscriptor.getStatSize();
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        return -1;
    }
}
