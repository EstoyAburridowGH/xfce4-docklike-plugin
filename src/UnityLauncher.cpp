#include "UnityLauncher.hpp"

#include <gio/gio.h>

namespace UnityLauncher {
	static GDBusConnection* mConnection = nullptr;
	static guint mSignalId = 0;
	static UpdateCallback mCallback = nullptr;

	static void onUnitySignal(GDBusConnection* connection, const gchar* sender_name, const gchar* object_path, const gchar* interface_name, const gchar* signal_name, GVariant* parameters, gpointer user_data) {
		const gchar* appUri;
		GVariant* propsVariant;

		g_variant_get(parameters, "(s@a{sv})", &appUri, &propsVariant);

		std::string uri(appUri);
		std::string desktopId;

		if (uri.rfind("application://", 0) == 0 && uri.size() > 14) {
			desktopId = uri.substr(14);
			std::string suffix = ".desktop";
			if (desktopId.size() > suffix.size() && desktopId.compare(desktopId.size() - suffix.size(), suffix.size(), suffix) == 0)
				desktopId = desktopId.substr(0, desktopId.size() - suffix.size());
		}

		if (!desktopId.empty()) {
			GVariantDict dict;
			g_variant_dict_init(&dict, propsVariant);

			int64_t count = 0;
			bool visible = false;

			if (g_variant_dict_contains(&dict, "count-visible")) {
				gboolean v = FALSE;
				g_variant_dict_lookup(&dict, "count-visible", "b", &v);
				visible = v;
			}

			if (g_variant_dict_contains(&dict, "count") && !g_variant_dict_lookup(&dict, "count", "x", &count)) {
				gint32 count32 = 0;
				if (g_variant_dict_lookup(&dict, "count", "i", &count32))
					count = count32;
			}

			g_variant_dict_clear(&dict);

			g_debug("[Unity] %s count=%ld visible=%d", desktopId.c_str(), (long)count, visible);

			if (mCallback) mCallback(desktopId, count, visible);
		}

		g_variant_unref(propsVariant);
	}

	void init() {
		mConnection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, nullptr);
		if (mConnection == nullptr) return;

		mSignalId = g_dbus_connection_signal_subscribe(mConnection, nullptr, "com.canonical.Unity.LauncherEntry", "Update", nullptr, nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onUnitySignal, nullptr, nullptr);
	}

	void finalize() {
		if (mConnection != nullptr && mSignalId > 0) {
			g_dbus_connection_signal_unsubscribe(mConnection, mSignalId);
			mSignalId = 0;
		}

		if (mConnection != nullptr) {
			g_object_unref(mConnection);
			mConnection = nullptr;
		}

		mCallback = nullptr;
	}

	void setUpdateCallback(UpdateCallback cb) { mCallback = std::move(cb); }
} // namespace UnityLauncher
