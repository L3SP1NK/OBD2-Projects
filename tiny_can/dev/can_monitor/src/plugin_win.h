#ifndef __PLUGIN_WIN_H__
#define __PLUGIN_WIN_H__


/*
typedef struct _PidginPluginUiInfo PidginPluginUiInfo;

//
// A GTK+ UI structure for plugins.
//
struct _PidginPluginUiInfo
{
	GtkWidget *(*get_config_frame)(PurplePlugin *plugin);

   int page_num;                                         // **< Reserved 

   // padding 
	void (*_pidgin_reserved1)(void);
	void (*_pidgin_reserved2)(void);
	void (*_pidgin_reserved3)(void);
	void (*_pidgin_reserved4)(void);
};

#define PIDGIN_PLUGIN_TYPE PIDGIN_UI

#define PIDGIN_IS_PIDGIN_PLUGIN(plugin) \
	((plugin)->info != NULL && (plugin)->info->ui_info != NULL && \
	 !strcmp((plugin)->info->ui_requirement, PIDGIN_PLUGIN_TYPE))

#define PIDGIN_PLUGIN_UI_INFO(plugin) \
   ((PidginPluginUiInfo *)(plugin)->info->ui_info) 
 */

/**
 * Returns the configuration frame widget for a GTK+ plugin, if one
 * exists.
 *
 * @param plugin The plugin.
 *
 * @return The frame, if the plugin is a GTK+ plugin and provides a
 *         configuration frame.
 */
// GtkWidget *pidgin_plugin_get_config_frame(PurplePlugin *plugin);


void PluginDialogShow(void);

#endif 
