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
    "run() - complex"
    >: (
      () => {
        let temp_dir = Util.get_temp_dir();
        let main_file = Filename.concat(temp_dir, "main.js");
        let app_file = Filename.concat(temp_dir, "App.js");
        let constants_file = Filename.concat(temp_dir, "common/constants.js");

        Build.run(
          ~catch=print_errs % Assert.fail,
          {
            ...__compiler_config,
            root_dir: "test/executable/.fixtures/complex",
            source_dir: "src",
          },
          {target: Target.(JavaScript(Common)), out_dir: temp_dir},
        );

        [main_file, app_file, constants_file]
        |> List.iter(Sys.file_exists % Assert.true_);

        main_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var TIMEOUT = require(\"./common/constants\");
var App = require(\"./App\");
var ABC = 123;
exports.ABC = ABC;
",
           );
        app_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var App = $knot.jsx.createTag(\"div\", {}, \"hello world\");
exports.App = App;
",
           );
        constants_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var TIMEOUT = 100;
exports.TIMEOUT = TIMEOUT;
",
           );
      }
    ),
  ];