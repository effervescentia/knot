open Kore;

include Test.Assert;

module Task = Executable.Task;
module ConfigFile = Executable.ConfigFile;
module ExecUtil = Executable.Util;

module Compare = {
  let global_config =
    Alcotest.testable(
      ppf =>
        ExecUtil.extract_global_config
        % Tuple.with_fst2("global")
        % ExecUtil.pp_attributes(ppf),
      (==),
    );

  let task = Alcotest.testable(Task.pp, (==));

  let config = Alcotest.testable(Config.pp, (==));

  let config_file_error =
    Alcotest.testable(
      ppf =>
        (
          fun
          | ConfigFile.CannotParse => "CannotParse"
          | ConfigFile.UnexpectedProperty(key) =>
            Fmt.str("UnexpectedProperty(%s)", key)
          | ConfigFile.InvalidFormat => "InvalidFormat"
        )
        % Fmt.pf(ppf, "%s"),
      (==),
    );
};

let task_with_config =
  Alcotest.(
    check(Compare.(pair(global_config, task)), "task and config file match")
  );

let config_file_result =
  Alcotest.(
    check(
      Compare.(result(config, config_file_error)),
      "config file result matches",
    )
  );
