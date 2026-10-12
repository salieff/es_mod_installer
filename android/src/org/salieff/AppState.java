package org.salieff;

public final class AppState {
    private static volatile boolean active = false;

    private AppState() {}

    public static void setActive(boolean value) {
        active = value;
    }

    public static boolean isActive() { return active; }
}
