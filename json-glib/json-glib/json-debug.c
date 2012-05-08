#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "json-debug.h"

static unsigned int json_debug_flags     = 0;
static gboolean     json_debug_flags_set = FALSE;

#ifdef JSON_ENABLE_DEBUG
static const GDebugKey json_debug_keys[] = {
  { "parser", JSON_DEBUG_PARSER },
  { "gobject", JSON_DEBUG_GOBJECT },
  { "path", JSON_DEBUG_PATH }
};
#endif /* JSON_ENABLE_DEBUG */

JsonDebugFlags
_json_get_debug_flags (void)
{
#ifdef JSON_ENABLE_DEBUG
  const gchar *env_str;

  if (json_debug_flags_set)
    return json_debug_flags;

  env_str = g_getenv ("JSON_DEBUG");
  if (env_str != NULL && *env_str != '\0')
    {
      json_debug_flags |= g_parse_debug_string (env_str,
                                                json_debug_keys,
                                                G_N_ELEMENTS (json_debug_keys));
    }

  json_debug_flags_set = TRUE;
#endif /* JSON_ENABLE_DEBUG */

  return json_debug_flags;
}
