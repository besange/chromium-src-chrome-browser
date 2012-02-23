include_rules = [
  "+chrome/app",
  "+chrome/app/locales",
  "+chrome/installer",
  "+chrome/personalization",
  "+chrome/profile_import",
  "+chrome/tools/profiles",  # For history unit tests.
  "+chrome/views",
  "+content/plugin/plugin_interpose_util_mac.h",
  "+content/public/browser",
  "+google_update",
  "+grit",  # For generated headers
  "+policy",  # For generated headers and source
  "+ppapi/c",  # For various types.
  "+ppapi/proxy",
  "+rlz",
  "+sandbox/linux",
  "+sandbox/src",  # The path doesn't say it, but this is the Windows sandbox.
  "+skia/ext",
  "+skia/include",
  "+third_party/cros",
  "+third_party/cros_system_api",
  "+webkit/database",
  "+webkit/forms",  # Defines some types that are marshalled over IPC.
  "+webkit/glue",  # Defines some types that are marshalled over IPC.
  "+webkit/plugins",  # Defines some types that are marshalled over IPC.
  "+webkit/quota",
  "+xib_localizers", # For generated mac localization helpers

  "-content/browser",
  # TODO(jam): Need to remove all these and use only content/public. BUG=98716
  # DO NOT ADD ANY MORE ITEMS TO THE LIST BELOW!
  "+content/browser/accessibility/browser_accessibility.h",
  "+content/browser/accessibility/browser_accessibility_manager.h",
  "+content/browser/browser_url_handler.h",
  "+content/browser/cert_store.h",
  "+content/browser/disposition_utils.h",
  "+content/browser/download/download_buffer.h",
  "+content/browser/download/download_create_info.h",
  "+content/browser/download/download_file_manager.h",
  "+content/browser/download/download_persistent_store_info.h",
  "+content/browser/download/download_request_handle.h",
  "+content/browser/download/download_state_info.h",
  "+content/browser/download/download_status_updater.h",
  "+content/browser/download/download_types.h",
  "+content/browser/download/drag_download_file.h",
  "+content/browser/download/drag_download_util.h",
  "+content/browser/download/interrupt_reasons.h",
  "+content/browser/download/mhtml_generation_manager.h",
  "+content/browser/find_pasteboard.h",
  "+content/browser/font_list_async.h",
  "+content/browser/geolocation/wifi_data_provider_common.h",
  "+content/browser/in_process_webkit/session_storage_namespace.h",
  "+content/browser/in_process_webkit/webkit_context.h",
  "+content/browser/intents/intent_injector.h",
  "+content/browser/load_notification_details.h",
  "+content/browser/mac/closure_blocks_leopard_compat.h",
  "+content/browser/mach_broker_mac.h",
  "+content/browser/mock_resource_context.h",
  "+content/browser/net/browser_online_state_observer.h",
  "+content/browser/net/url_request_failed_dns_job.h",
  "+content/browser/net/url_request_mock_http_job.h",
  "+content/browser/net/url_request_slow_download_job.h",
  "+content/browser/net/url_request_slow_http_job.h",
  "+content/browser/plugin_service_filter.h",
  "+content/browser/quota_permission_context.h",
  "+content/browser/renderer_host/backing_store.h",
  "+content/browser/renderer_host/backing_store_gtk.h",
  "+content/browser/renderer_host/backing_store_mac.h",
  "+content/browser/renderer_host/backing_store_manager.h",
  "+content/browser/renderer_host/dummy_resource_handler.h",
  "+content/browser/renderer_host/media/media_observer.h",
  "+content/browser/renderer_host/mock_render_process_host.h",
  "+content/browser/renderer_host/render_process_host_browsertest.h",
  "+content/browser/renderer_host/render_sandbox_host_linux.h",
  "+content/browser/renderer_host/render_view_host.h",
  "+content/browser/renderer_host/render_view_host_factory.h",
  "+content/browser/renderer_host/render_widget_host.h",
  "+content/browser/renderer_host/render_widget_host_view_mac_delegate.h",
  "+content/browser/renderer_host/resource_dispatcher_host.h",
  "+content/browser/renderer_host/resource_dispatcher_host_request_info.h",
  "+content/browser/renderer_host/resource_queue.h",
  "+content/browser/renderer_host/resource_request_details.h",
  "+content/browser/renderer_host/test_render_view_host.h",
  "+content/browser/speech/speech_input_manager.h",
  "+content/browser/speech/speech_recognizer.h",
  "+content/browser/tab_contents/popup_menu_helper_mac.h",
  "+content/browser/tab_contents/provisional_load_details.h",
  "+content/browser/tab_contents/tab_contents_view_gtk.h",
  "+content/browser/tab_contents/tab_contents_view_helper.h",
  "+content/browser/tab_contents/tab_contents_view_wrapper_gtk.h",
  "+content/browser/tab_contents/test_tab_contents.h",
  "+content/browser/tab_contents/title_updated_details.h",
  "+content/browser/tab_contents/web_contents_view_android.h",
  "+content/browser/tab_contents/web_contents_view_mac.h",
  "+content/browser/tab_contents/web_drag_dest_delegate.h",
  "+content/browser/tab_contents/web_drag_dest_gtk.h",
  "+content/browser/tab_contents/web_drag_dest_mac.h",
  "+content/browser/tab_contents/web_drag_source_gtk.h",
  "+content/browser/tab_contents/web_drag_source_mac.h",
  "+content/browser/trace_controller.h",
  "+content/browser/zygote_host_linux.h",
  # DO NOT ADD ANY MORE ITEMS TO THE ABOVE LIST!

  "-content/common",

  # Other libraries.
  "+chrome/third_party/hunspell",
  "+chrome/third_party/mozilla_security_manager",
  "+libxml",  # For search engine definition parsing.
  "+third_party/apple_sample_code",  # Apple code ImageAndTextCell.
  "+third_party/cld",
  "+third_party/expat",
  "+third_party/iaccessible2",
  "+third_party/isimpledom",
  "+third_party/libevent",  # For the remote V8 debugging server
  "+third_party/libjingle",
  "+third_party/protobuf/src/google/protobuf",
  "+third_party/sqlite",
  "+third_party/undoview",
  "+v8/include",  # Browser uses V8 to get the version and run the debugger.

  # FIXME: these should probably not be here, we need to find a better
  # structure for these includes.
  "+chrome/renderer",
]
