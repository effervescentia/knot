/**
 Usage pretty printers.
 */
open Kore;

let commands = [
  (Task.build.name, " compile files to target in output directory"),
  (Task.watch.name, " run build and incrementally rebuild changed files"),
  (Task.format.name, " update code style and spacing"),
  (Task.lint.name, " analyze code style and report on anti-patterns"),
  (Task.bundle.name, " generate executable from source code"),
  (
    Task.dev_serve.name,
    " run a development server to enable continuous development",
  ),
  (
    Task.lang_serve.name,
    " run an LSP-compliant server for integration with IDEs",
  ),
  (
    Task.build_serve.name,
    " run a JSONRPC server that can perform incremental compilation",
  ),
];

let _pp_section = (pp_value: Fmt.t('a)): Fmt.t((string, 'a)) =>
  Fmt.(
    (ppf, (label, value)) =>
      pf(ppf, "%a@,%a", bold_str, label, indented(pp_value), value)
  );

let _pp_argument_list = cfg =>
  Fmt.(list(~sep=Sep.trailing_double_newline, Argument.pp(cfg)));

let _pp_sub_command: Fmt.t(Command.t('a)) =
  Fmt.(
    (ppf, {name}) =>
      pf(
        ppf,
        "  %a [options]",
        bold(ppf => pf(ppf, "%s %s", binary_name)),
        name,
      )
  );

let pp_command_list = (ppf, cmds) => {
  let offset =
    cmds
    |> List.map(((x, _)) => String.length(x))
    |> List.fold_left((acc, len) => len > acc ? len : acc, 0)
    |> (+)(2);

  Fmt.(
    list(
      ~sep=Sep.trailing_newline,
      (ppf, (cmd, desc)) =>
        pf(ppf, "%a%s", bold(ppf => pf(ppf, "%-*s", offset)), cmd, desc),
      ppf,
      cmds,
    )
  );
};

let _pp_cmd_usage = (static_cfg): Fmt.t(option(Command.t(Task.t))) =>
  Fmt.(
    ppf =>
      fun
      | None =>
        pf(
          ppf,
          "  %a <command> ...@,@,%a",
          bold_str,
          binary_name,
          _pp_section(pp_command_list),
          ("COMMANDS", commands),
        )

      | Some(Command.{name, arguments: cmd_args} as cmd) =>
        pf(
          ppf,
          "%a@,%a",
          _pp_sub_command,
          cmd,
          ppf =>
            fun
            | [] => nop(ppf, ())
            | args =>
              pf(
                ppf,
                "@,%a",
                _pp_section(_pp_argument_list(static_cfg)),
                ("COMMAND OPTIONS", args),
              ),
          cmd_args,
        )
  );

let rec pp:
  Fmt.t((option(Command.t(Task.t)), option(Config.t), list(Argument.t))) =
  Fmt.(
    (ppf, (cmd, static_cfg, opts)) =>
      root(
        ppf =>
          pf(
            ppf,
            "%a@,%a",
            _pp_cmd_usage(static_cfg),
            cmd,
            _pp_section(_pp_argument_list(static_cfg)),
          ),
        ppf,
        ("OPTIONS", opts),
      )
  );
