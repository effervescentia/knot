open Kore;

type value_set_t('a) = {value_set: 'a};

type symbol_t =
  | File
  | Module
  | Namespace
  | Package
  | Class
  | Method
  | Property
  | Field
  | Constructor
  | Enum
  | Interface
  | Function
  | Variable
  | Constant
  | String
  | Number
  | Boolean
  | Array
  | Object
  | Key
  | Null
  | EnumMember
  | Struct
  | Event
  | Operator
  | TypeParameter;

type symbol_tag_t =
  | Deprecated;

type with_dynamic_registration_t = {dynamic_registration: option(bool)};

type with_refresh_support_t = {refresh_support: option(bool)};

type with_link_support_t = {
  dynamic_registration: option(bool),
  link_support: option(bool),
};

module Workspace = {
  type resource_operation_t =
    | Create
    | Rename
    | Delete;

  type failure_handling_t =
    | Abort
    | Transactional
    | Undo
    | TextOnlyTransactional;

  type change_annotation_support_t = {groups_on_label: option(bool)};

  type workspace_edit_capabilities_t = {
    document_changes: option(bool),
    resource_operations: option(list(resource_operation_t)),
    failure_handling: option(failure_handling_t),
    normalizes_line_endings: option(bool),
    change_annotation_support: option(change_annotation_support_t),
  };

  type symbol_capabilities_t = {
    dynamic_registration: option(bool),
    symbol_kind: option(value_set_t(option(list(symbol_t)))),
    tag_support: option(value_set_t(list(symbol_tag_t))),
  };

  type file_operations_capabilities_t = {
    dynamic_registration: option(bool),
    did_create: option(bool),
    will_create: option(bool),
    did_rename: option(bool),
    will_rename: option(bool),
    did_delete: option(bool),
    will_delete: option(bool),
  };

  type t = {
    apply_edit: option(bool),
    workspace_edit: option(workspace_edit_capabilities_t),
    did_change_configuration: option(with_dynamic_registration_t),
    did_change_watched_files: option(with_dynamic_registration_t),
    symbol: option(symbol_capabilities_t),
    execute_command: option(with_dynamic_registration_t),
    workspace_folders: option(bool),
    configuration: option(bool),
    semantic_tokens: option(with_refresh_support_t),
    code_lens: option(with_refresh_support_t),
    file_operations: option(file_operations_capabilities_t),
  };
};

module Window = {
  type message_action_item_t = {additional_properties_support: option(bool)};

  type show_message_capabilities_t = {
    message_action_item: option(message_action_item_t),
  };

  type show_document_capabilities_t = {support: bool};

  type t = {
    work_done_progress: option(bool),
    show_message: option(show_message_capabilities_t),
    show_document: option(show_document_capabilities_t),
  };
};

module General = {
  type regular_expressions_capabilities_t = {
    engine: string,
    version: option(string),
  };

  type markdown_capabilities_t = {
    parser: string,
    version: option(string),
  };

  type t = {
    regular_expressions: option(regular_expressions_capabilities_t),
    markdown: option(markdown_capabilities_t),
  };
};

module TextDocument = {
  type synchronization_capabilities_t = {
    dynamic_registration: option(bool),
    will_save: option(bool),
    will_save_until: option(bool),
    did_save: option(bool),
  };

  type properties_t('a) = {properties: list('a)};

  type resolve_support_t = properties_t(string);

  type markup_t =
    | PlainText
    | Markdown;

  type completion_item_tag_t =
    | Deprecated;

  type insert_text_mode_t =
    | AsIs
    | AdjustIndentation;

  type completion_item_capabilities_t = {
    snippet_support: option(bool),
    commit_characters_support: option(bool),
    documentation_format: option(list(markup_t)),
    deprecated_support: option(bool),
    preselect_support: option(bool),
    tag_support: option(value_set_t(list(completion_item_tag_t))),
    insert_replace_support: option(bool),
    resolve_support: option(resolve_support_t),
    insert_text_mode_support: option(value_set_t(list(insert_text_mode_t))),
  };

  type completion_item_t =
    | Text
    | Method
    | Function
    | Constructor
    | Field
    | Variable
    | Class
    | Interface
    | Module
    | Property
    | Unit
    | Value
    | Enum
    | Keyword
    | Snippet
    | Color
    | File
    | Reference
    | Folder
    | EnumMember
    | Constant
    | Struct
    | Event
    | Operator
    | TypeParameter;

  type completion_capabilities_t = {
    dynamic_registration: option(bool),
    completion_item: option(completion_item_capabilities_t),
    completion_item_kind:
      option(value_set_t(option(list(completion_item_t)))),
    context_support: option(bool),
  };

  type hover_capabilities_t = {
    dynamic_registration: option(bool),
    content_format: option(list(markup_t)),
  };

  type parameter_information_t = {label_offset_support: option(bool)};

  type signature_information_t = {
    documentation_format: option(list(markup_t)),
    parameter_information: option(parameter_information_t),
    active_parameter_support: option(bool),
  };

  type signature_help_capabilities_t = {
    dynamic_registration: option(bool),
    signature_information: option(signature_information_t),
    context_support: option(bool),
  };

  type document_symbol_capabilities_t = {
    dynamic_registration: option(bool),
    symbol_kind: option(value_set_t(option(list(symbol_t)))),
    hierarchical_document_symbol_support: option(bool),
    tag_support: option(value_set_t(list(symbol_tag_t))),
    label_support: option(bool),
  };

  type code_action_t =
    | Empty
    | QuickFix
    | Refactor
    | RefactorExtract
    | RefactorInline
    | RefactorRewrite
    | SourceOrganizeImports;

  type code_action_literal_support_t = {
    code_action_kind: value_set_t(list(code_action_t)),
  };

  type code_action_capabilities_t = {
    dynamic_registration: option(bool),
    code_action_literal_support: option(code_action_literal_support_t),
    is_preferred_support: option(bool),
    disabled_support: option(bool),
    data_support: option(bool),
    resolve_support: option(properties_t(string)),
    honors_change_annotations: option(bool),
  };

  type document_link_capabilities_t = {
    dynamic_registration: option(bool),
    tooltip_support: option(bool),
  };

  type prepare_support_default_behavior_t =
    | Identifier;

  type rename_capabilities_t = {
    dynamic_registration: option(bool),
    prepare_support: option(bool),
    prepare_support_default_behavior:
      option(prepare_support_default_behavior_t),
    honors_change_annotations: option(bool),
  };

  type diagnostic_tag_t =
    | Unnecessary
    | Deprecated;

  type publish_diagnostics_capabilities_t = {
    related_information: option(bool),
    tag_support: value_set_t(list(diagnostic_tag_t)),
    version_support: option(bool),
    code_description_support: option(bool),
    data_support: option(bool),
  };

  type folding_range_capabilities_t = {
    dynamic_registration: option(bool),
    range_limit: option(int),
    line_folding_only: option(bool),
  };

  type token_format_t =
    | Relative;

  type full_options_t = {delta: option(bool)};

  type full_t =
    | Bool(bool)
    | Options(full_options_t);

  type semantic_tokens_requests_t = {
    range: option(bool),
    full: option(full_t),
  };

  type semantic_tokens_capabilities_t = {
    dynamic_registration: option(bool),
    requests: semantic_tokens_requests_t,
    token_types: list(string),
    token_modifiers: list(string),
    formats: list(token_format_t),
    overlapping_token_support: option(bool),
    multiline_token_support: option(bool),
  };

  type t = {
    synchronization: option(synchronization_capabilities_t),
    completion: option(completion_capabilities_t),
    hover: option(hover_capabilities_t),
    signature_help: option(signature_help_capabilities_t),
    declaration: option(with_link_support_t),
    definition: option(with_link_support_t),
    type_definition: option(with_link_support_t),
    implementation: option(with_link_support_t),
    document_highlight: option(with_dynamic_registration_t),
    document_symbol: option(document_symbol_capabilities_t),
    code_action: option(code_action_capabilities_t),
    code_lens: option(with_dynamic_registration_t),
    document_link: option(document_link_capabilities_t),
    color_provider: option(with_dynamic_registration_t),
    formatting: option(with_dynamic_registration_t),
    range_formatting: option(with_dynamic_registration_t),
    on_type_formatting: option(with_dynamic_registration_t),
    rename: option(rename_capabilities_t),
    publish_diagnostics: option(publish_diagnostics_capabilities_t),
    folding_range: option(folding_range_capabilities_t),
    selection_range: option(with_dynamic_registration_t),
    linked_editing_range: option(with_dynamic_registration_t),
    call_hierarchy: option(with_dynamic_registration_t),
    semantic_tokens: option(semantic_tokens_capabilities_t),
    moniker: option(with_dynamic_registration_t),
  };
};

type t = {
  workspace: option(Workspace.t),
  text_document: option(TextDocument.t),
  window: option(Window.t),
  general: option(General.t),
};

open Yojson.Basic.Util;

module Deserialize = {
  let _get_dynamic_registration =
    member("dynamicRegistration") % to_bool_option;
  let _get_refresh_support = member("refreshSupport") % to_bool_option;

  let to_symbol =
    fun
    | `Int(1) => Some(File)
    | `Int(2) => Some(Module)
    | `Int(3) => Some(Namespace)
    | `Int(4) => Some(Package)
    | `Int(5) => Some(Class)
    | `Int(6) => Some(Method)
    | `Int(7) => Some(Property)
    | `Int(8) => Some(Field)
    | `Int(9) => Some(Constructor)
    | `Int(10) => Some(Enum)
    | `Int(11) => Some(Interface)
    | `Int(12) => Some(Function)
    | `Int(13) => Some(Variable)
    | `Int(14) => Some(Constant)
    | `Int(15) => Some(String)
    | `Int(16) => Some(Number)
    | `Int(17) => Some(Boolean)
    | `Int(18) => Some(Array)
    | `Int(19) => Some(Object)
    | `Int(20) => Some(Key)
    | `Int(21) => Some(Null)
    | `Int(22) => Some(EnumMember)
    | `Int(23) => Some(Struct)
    | `Int(24) => Some(Event)
    | `Int(25) => Some(Operator)
    | `Int(26) => Some(TypeParameter)
    | _ => None;

  let to_symbol_tag =
    fun
    | `Int(1) => Some(Deprecated)
    | _ => None;

  let to_dynamic_registration =
    fun
    | `Assoc(_) as x => {
        let dynamic_registration = x |> _get_dynamic_registration;

        Some({dynamic_registration: dynamic_registration});
      }
    | _ => None;

  let to_refresh_support =
    fun
    | `Assoc(_) as x => {
        let refresh_support = x |> _get_refresh_support;

        Some({refresh_support: refresh_support});
      }
    | _ => None;

  let to_value_set = f =>
    fun
    | `Assoc(_) as x => {
        let value_set = x |> member("valueSet") |> f;

        Some({value_set: value_set});
      }
    | _ => None;

  let workspace =
    Workspace.(
      fun
      | `Assoc(_) as x => {
          let apply_edit = x |> member("applyEdit") |> to_bool_option;
          let workspace_folders =
            x |> member("workspaceFolders") |> to_bool_option;
          let configuration = x |> member("configuration") |> to_bool_option;
          let did_change_configuration =
            x |> member("didChangeConfiguration") |> to_dynamic_registration;
          let did_change_watched_files =
            x |> member("didChangeWatchedFiles") |> to_dynamic_registration;
          let execute_command =
            x |> member("executeCommand") |> to_dynamic_registration;
          let semantic_tokens =
            x |> member("semanticTokens") |> to_refresh_support;
          let code_lens = x |> member("codeLens") |> to_refresh_support;

          let workspace_edit =
            x
            |> member("workspaceEdit")
            |> (
              fun
              | `Assoc(_) as x => {
                  let document_changes =
                    x |> member("documentChanges") |> to_bool_option;
                  let normalizes_line_endings =
                    x |> member("normalizesLineEndings") |> to_bool_option;

                  let resource_operations =
                    x
                    |> member("resourceOperations")
                    |> (
                      fun
                      | `List(xs) =>
                        xs
                        |> List.filter_map(
                             fun
                             | `String("create") => Some(Create)
                             | `String("rename") => Some(Rename)
                             | `String("delete") => Some(Delete)
                             | _ => None,
                           )
                        |> Option.some
                      | _ => None
                    );

                  let failure_handling =
                    x
                    |> member("failureHandling")
                    |> (
                      fun
                      | `String("abort") => Some(Abort)
                      | `String("transactional") => Some(Transactional)
                      | `String("undo") => Some(Undo)
                      | `String("textOnlyTransactional") =>
                        Some(TextOnlyTransactional)
                      | _ => None
                    );

                  let change_annotation_support =
                    x
                    |> member("changeAnnotationSupport")
                    |> (
                      fun
                      | `Assoc(_) as x => {
                          let groups_on_label =
                            x |> member("groupsOnLabel") |> to_bool_option;

                          Some({groups_on_label: groups_on_label});
                        }
                      | _ => None
                    );

                  Some({
                    document_changes,
                    resource_operations,
                    failure_handling,
                    normalizes_line_endings,
                    change_annotation_support,
                  });
                }
              | _ => None
            );

          let symbol =
            x
            |> member("symbol")
            |> (
              fun
              | `Assoc(_) as x => {
                  let dynamic_registration = x |> _get_dynamic_registration;

                  let symbol_kind =
                    x
                    |> member("symbolKind")
                    |> to_value_set(
                         fun
                         | `List(xs) =>
                           xs
                           |> List.map(to_symbol % Option.get)
                           |> Option.some
                         | _ => None,
                       );

                  let tag_support =
                    x
                    |> member("tagSupport")
                    |> to_value_set(
                         fun
                         | `List(xs) =>
                           xs |> List.map(to_symbol_tag % Option.get)
                         | x => raise(Type_error("tagSupport", x)),
                       );

                  Some({dynamic_registration, symbol_kind, tag_support});
                }
              | _ => None
            );

          let file_operations =
            x
            |> member("file_operations")
            |> (
              fun
              | `Assoc(_) as x => {
                  let dynamic_registration =
                    x |> member("dynamicRegistration") |> to_bool_option;
                  let did_create = x |> member("didCreate") |> to_bool_option;
                  let will_create =
                    x |> member("willCreate") |> to_bool_option;
                  let did_rename = x |> member("didRename") |> to_bool_option;
                  let will_rename =
                    x |> member("willRename") |> to_bool_option;
                  let did_delete = x |> member("didDelete") |> to_bool_option;
                  let will_delete =
                    x |> member("willDelete") |> to_bool_option;

                  Some({
                    dynamic_registration,
                    did_create,
                    will_create,
                    did_rename,
                    will_rename,
                    did_delete,
                    will_delete,
                  });
                }
              | _ => None
            );

          Some({
            apply_edit,
            workspace_edit,
            did_change_configuration,
            did_change_watched_files,
            symbol,
            execute_command,
            workspace_folders,
            configuration,
            semantic_tokens,
            code_lens,
            file_operations,
          });
        }
      | _ => None
    );

  let text_document =
    TextDocument.(
      fun
      | `Assoc(_) as x => {
          let document_highlight =
            x |> member("documentHighlight") |> to_dynamic_registration;
          let code_lens = x |> member("code_lens") |> to_dynamic_registration;
          let color_provider =
            x |> member("colorProvider") |> to_dynamic_registration;
          let formatting =
            x |> member("formatting") |> to_dynamic_registration;
          let range_formatting =
            x |> member("rangeFormatting") |> to_dynamic_registration;
          let on_type_formatting =
            x |> member("onTypeFormatting") |> to_dynamic_registration;
          let selection_range =
            x |> member("selectionRange") |> to_dynamic_registration;
          let linked_editing_range =
            x |> member("linkedEditingRange") |> to_dynamic_registration;
          let call_hierarchy =
            x |> member("callHierarchy") |> to_dynamic_registration;
          let moniker = x |> member("moniker") |> to_dynamic_registration;

          let synchronization =
            x
            |> member("synchronization")
            |> (
              fun
              | `Assoc(_) as x => {
                  let dynamic_registration =
                    x |> member("dynamicRegistration") |> to_bool_option;
                  let will_save = x |> member("willSave") |> to_bool_option;
                  let will_save_until =
                    x |> member("willSaveUntil") |> to_bool_option;
                  let did_save = x |> member("didSave") |> to_bool_option;

                  Some({
                    dynamic_registration,
                    will_save,
                    will_save_until,
                    did_save,
                  });
                }
              | _ => None
            );

          let document_link =
            x
            |> member("documentLink")
            |> (
              fun
              | `Assoc(_) as x => {
                  let dynamic_registration = x |> _get_dynamic_registration;
                  let tooltip_support =
                    x |> member("tooltipSupport") |> to_bool_option;

                  Some({dynamic_registration, tooltip_support});
                }
              | _ => None
            );

          let folding_range =
            x
            |> member("foldingRange")
            |> (
              fun
              | `Assoc(_) as x => {
                  let dynamic_registration = x |> _get_dynamic_registration;
                  let range_limit =
                    x |> member("rangeLimit") |> to_int_option;
                  let line_folding_only =
                    x |> member("lineFoldingOnly") |> to_bool_option;

                  Some({
                    dynamic_registration,
                    range_limit,
                    line_folding_only,
                  });
                }
              | _ => None
            );

          /* completion: option(completion_capabilities_t),
             hover: option(hover_capabilities_t),
             signature_help: option(signature_help_capabilities_t),
             declaration: option(with_link_support_t),
             definition: option(with_link_support_t),
             type_definition: option(with_link_support_t),
             implementation: option(with_link_support_t),
             document_symbol: option(document_symbol_capabilities_t),
             code_action: option(code_action_capabilities_t),
             rename: option(rename_capabilities_t),
             publish_diagnostics: option(publish_diagnostics_capabilities_t),
             semantic_tokens: option(semantic_tokens_capabilities_t), */

          Some({
            synchronization,
            completion: None,
            hover: None,
            signature_help: None,
            declaration: None,
            definition: None,
            type_definition: None,
            implementation: None,
            document_highlight,
            document_symbol: None,
            code_action: None,
            code_lens,
            document_link,
            color_provider,
            formatting,
            range_formatting,
            on_type_formatting,
            rename: None,
            publish_diagnostics: None,
            folding_range,
            selection_range,
            linked_editing_range,
            call_hierarchy,
            semantic_tokens: None,
            moniker,
          });
        }
      | _ => None
    );

  let window =
    Window.(
      fun
      | `Assoc(_) as x => {
          let work_done_progress =
            x |> member("workDoneProgress") |> to_bool_option;

          let show_message =
            x
            |> member("showMessage")
            |> (
              fun
              | `Assoc(_) as x => {
                  let message_action_item =
                    x
                    |> member("messageActionItem")
                    |> (
                      fun
                      | `Assoc(_) as x => {
                          let additional_properties_support =
                            x
                            |> member("additionalPropertiesSupport")
                            |> to_bool_option;

                          Some({
                            additional_properties_support: additional_properties_support,
                          });
                        }
                      | _ => None
                    );

                  Some({message_action_item: message_action_item});
                }
              | _ => None
            );

          let show_document =
            x
            |> member("showDocument")
            |> (
              fun
              | `Assoc(_) as x => {
                  let support = x |> member("support") |> to_bool;

                  Some({support: support});
                }
              | _ => None
            );

          Some({work_done_progress, show_message, show_document});
        }
      | _ => None
    );

  let general =
    General.(
      fun
      | `Assoc(_) as x => {
          let regular_expressions =
            x
            |> member("regularExpressions")
            |> (
              fun
              | `Assoc(_) as x => {
                  let engine = x |> member("engine") |> to_string;
                  let version = x |> member("version") |> to_string_option;

                  Some({engine, version});
                }
              | _ => None
            );

          let markdown =
            x
            |> member("markdown")
            |> (
              fun
              | `Assoc(_) as x => {
                  let parser = x |> member("parser") |> to_string;
                  let version = x |> member("version") |> to_string_option;

                  Some({parser, version});
                }
              | _ => None
            );

          Some({regular_expressions, markdown});
        }
      | _ => None
    );
};

let deserialize =
  fun
  | `Assoc(_) as x => {
      let workspace = x |> member("workspace") |> Deserialize.workspace;
      let text_document =
        x |> member("textDocument") |> Deserialize.text_document;
      let window = x |> member("window") |> Deserialize.window;
      let general = x |> member("general") |> Deserialize.general;

      Some({workspace, text_document, window, general});
    }
  | _ => None;
