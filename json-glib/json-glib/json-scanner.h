/* json-scanner.h: Tokenizer for JSON
 *
 * This file is part of JSON-GLib
 * Copyright (C) 2008 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * JsonScanner is a specialized tokenizer for JSON adapted from
 * the GScanner tokenizer in GLib; GScanner came with this notice:
 * 
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 *
 * JsonScanner: modified by Emmanuele Bassi <ebassi@openedhand.com>
 */

#ifndef __JSON_SCANNER_H__
#define __JSON_SCANNER_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _JsonScanner       JsonScanner;
typedef struct _JsonScannerConfig JsonScannerConfig;

typedef void (* JsonScannerMsgFunc) (JsonScanner *scanner,
                                     gchar       *message,
                                     gboolean     is_error);

/**
 * JsonTokenType:
 * @JSON_TOKEN_INVALID: marker
 * @JSON_TOKEN_TRUE: symbol for 'true' bareword
 * @JSON_TOKEN_FALSE: symbol for 'false' bareword
 * @JSON_TOKEN_NULL: symbol for 'null' bareword
 * @JSON_TOKEN_VAR: symbol for 'var' bareword
 * @JSON_TOKEN_LAST: marker
 *
 * Tokens for JsonScanner-based parser, extending #GTokenType.
 */
typedef enum {
  JSON_TOKEN_INVALID = G_TOKEN_LAST,

  JSON_TOKEN_TRUE,
  JSON_TOKEN_FALSE,
  JSON_TOKEN_NULL,
  JSON_TOKEN_VAR,

  JSON_TOKEN_LAST
} JsonTokenType;

/**
 * JsonScanner:
 *
 * Tokenizer scanner for JSON. See #GScanner
 *
 * Since: 0.6
 */
struct _JsonScanner
{
  /*< private >*/
  /* unused fields */
  gpointer user_data;
  guint max_parse_errors;
  
  /* json_scanner_error() increments this field */
  guint parse_errors;
  
  /* name of input stream, featured by the default message handler */
  const gchar *input_name;
  
  /* quarked data */
  GData *qdata;
  
  /* link into the scanner configuration */
  JsonScannerConfig *config;
  
  /* fields filled in after json_scanner_get_next_token() */
  GTokenType token;
  GTokenValue value;
  guint line;
  guint position;
  
  /* fields filled in after json_scanner_peek_next_token() */
  GTokenType next_token;
  GTokenValue next_value;
  guint next_line;
  guint next_position;
  
  /* to be considered private */
  GHashTable *symbol_table;
  gint input_fd;
  const gchar *text;
  const gchar *text_end;
  gchar *buffer;
  guint scope_id;
  
  /* handler function for _warn and _error */
  JsonScannerMsgFunc msg_handler;
};

JsonScanner *json_scanner_new                  (void);
void         json_scanner_destroy              (JsonScanner *scanner);
void         json_scanner_input_file           (JsonScanner *scanner,
                                                gint         input_fd);
void         json_scanner_sync_file_offset     (JsonScanner *scanner);
void         json_scanner_input_text           (JsonScanner *scanner,
                                                const gchar *text,
                                                guint        text_len);
GTokenType   json_scanner_get_next_token       (JsonScanner *scanner);
GTokenType   json_scanner_peek_next_token      (JsonScanner *scanner);
GTokenType   json_scanner_cur_token            (JsonScanner *scanner);
GTokenValue  json_scanner_cur_value            (JsonScanner *scanner);
guint        json_scanner_cur_line             (JsonScanner *scanner);
guint        json_scanner_cur_position         (JsonScanner *scanner);
gboolean     json_scanner_eof                  (JsonScanner *scanner);
guint        json_scanner_set_scope            (JsonScanner *scanner,
                                                guint        scope_id);
void         json_scanner_scope_add_symbol     (JsonScanner *scanner,
                                                guint        scope_id,
                                                const gchar *symbol,
                                                gpointer     value);
void         json_scanner_scope_remove_symbol  (JsonScanner *scanner,
                                                guint        scope_id,
                                                const gchar *symbol);
gpointer     json_scanner_scope_lookup_symbol  (JsonScanner *scanner,
                                                guint        scope_id,
                                                const gchar *symbol);
void         json_scanner_scope_foreach_symbol (JsonScanner *scanner,
                                                guint        scope_id,
                                                GHFunc       func,
                                                gpointer     user_data);
gpointer     json_scanner_lookup_symbol        (JsonScanner *scanner,
                                                const gchar *symbol);
void         json_scanner_unexp_token          (JsonScanner *scanner,
                                                GTokenType   expected_token,
                                                const gchar *identifier_spec,
                                                const gchar *symbol_spec,
                                                const gchar *symbol_name,
                                                const gchar *message,
                                                gint         is_error);
void         json_scanner_error                (JsonScanner *scanner,
                                                const gchar *format,
                                                ...) G_GNUC_PRINTF (2,3);
void         json_scanner_warn                 (JsonScanner *scanner,
                                                const gchar *format,
                                                ...) G_GNUC_PRINTF (2,3);

G_END_DECLS

#endif /* __JSON_SCANNER_H__ */
