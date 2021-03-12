open Kore;

module Build = Executable.Build;

let __compiler_config =
  Compile.Compiler.{
    name: "foo",
    entry: Internal("main"),
    root_dir: "test/executable/.fixtures/simple",
    source_dir: ".",
  };

let suite =
  "Executable.Build"
  >::: [
    "run() - simple"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let output_file = Filename.concat(temp_dir, "main.js");

        Build.run(
          ~catch=print_errs % Assert.fail,
          __compiler_config,
          {target: Target.(JavaScript(Common)), out_dir: temp_dir},
        );

        output_file |> Sys.file_exists |> Assert.true_;
        output_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var ABC = 123;
exports.ABC = ABC;
",
           );
      }
    ),
  ];
