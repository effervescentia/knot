open Kore;

module ConfigFile = Executable.ConfigFile;

let __empty_config_file = Filename.concat(no_target_fixture_dir, ".knot.yml");

let suite =
  "Executable.ConfigFile"
  >::: [
    "find() - local to working directory"
    >: (
      () =>
        Assert.opt_string(
          Some(__empty_config_file),
          ConfigFile.find(no_target_fixture_dir),
        )
    ),
    "find() - in parent directory"
    >: (
      () =>
        Assert.opt_string(
          Some(__empty_config_file),
          ConfigFile.find(Filename.concat(no_target_fixture_dir, "src")),
        )
    ),
    "read() - empty config file"
    >: (
      () =>
        Assert.config_file_result(
          Ok(Config.defaults),
          ConfigFile.read(__empty_config_file),
        )
    ),
    "read() - full config file"
    >: (
      () =>
        Assert.config_file_result(
          Ok(
            Config.{
              name: Some("foobar_prototype"),
              root_dir: "code",
              source_dir: "lib",
              out_dir: "output",
              entry: "index.kn",
              target: Some(Target.JavaScript(Common)),
              color: true,
              fix: true,
              fail_fast: true,
              log_imports: true,
              debug: true,
              port: 1338,
            },
          ),
          ConfigFile.read(full_config_file),
        )
    ),
    "read() - unexpected property"
    >: (
      () =>
        Assert.config_file_result(
          Error(UnexpectedProperty("foo")),
          ConfigFile.read(unexpected_prop_config_file),
        )
    ),
    "read() - invalid format"
    >: (
      () =>
        Assert.config_file_result(
          Error(InvalidFormat),
          ConfigFile.read(invalid_format_config_file),
        )
    ),
    "read() - cannot parse"
    >: (
      () =>
        Assert.config_file_result(
          Error(CannotParse),
          ConfigFile.read(cannot_parse_config_file),
        )
    ),
  ];
