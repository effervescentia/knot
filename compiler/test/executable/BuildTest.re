open Kore;

module Build = Executable.Build;

let __entry = Reference.Namespace.Internal("main");
let __compiler_config =
  Executable.Kore.{
    name: "foo",
    root_dir: simple_fixture_dir,
    source_dir: ".",
    debug: false,
    color: false,
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
          ~report=_ => Util.print_errs % Assert.fail,
          __compiler_config,
          {
            target: Target.(JavaScript(Common)),
            out_dir: temp_dir,
            entry: __entry,
            fail_fast: true,
          },
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
          ~report=_ => Util.print_errs % Assert.fail,
          {
            ...__compiler_config,
            root_dir: complex_fixture_dir,
            source_dir: "src",
          },
          {
            target: Target.(JavaScript(Common)),
            out_dir: temp_dir,
            entry: __entry,
            fail_fast: true,
          },
        );

        [main_file, app_file, constants_file]
        |> List.iter(Sys.file_exists % Assert.true_);

        main_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var $import$_$common$constants = require(\"./common/constants\");
var TIMEOUT = $import$_$common$constants.main;
$import$_$common$constants = null;
var $import$_$App = require(\"./App\");
var App = $import$_$App.main;
$import$_$App = null;
var ABC = 123;
exports.ABC = ABC;
",
           );
        app_file
        |> Util.read_file_to_string
        |> Assert.string(
             "var $knot = require(\"@knot/runtime\");
var App = $knot.jsx.createTag(\"div\", null, \"hello world\");
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
