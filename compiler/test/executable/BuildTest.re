open Kore;

module Build = Executable.Build;

let __entry = Reference.Namespace.Internal("main");
let __compiler_config =
  Config.{
    name: "foo",
    debug: false,
    color: false,
    working_dir: ".",
    stdlib: empty_library_file,
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
          ~report=_ => ~@AST.Error.pp_dump_err_list % Assert.fail,
          __compiler_config,
          {
            target: Target.(JavaScript(Common)),
            root_dir: simple_fixture_dir,
            source_dir: ".",
            out_dir: temp_dir,
            entry: __entry,
            fail_fast: true,
            log_imports: false,
            ambient: empty_library_file,
          },
        );

        Assert.file_exists(output_file);
        Assert.string(
          "var $knot = require(\"@knot/runtime\");
var ABC = 123;
exports.ABC = ABC;
",
          Util.read_file_to_string(output_file),
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
          ~report=_ => ~@AST.Error.pp_dump_err_list % Assert.fail,
          __compiler_config,
          {
            target: Target.(JavaScript(Common)),
            root_dir: complex_fixture_dir,
            source_dir: "src",
            out_dir: temp_dir,
            entry: __entry,
            fail_fast: true,
            log_imports: false,
            ambient: empty_library_file,
          },
        );

        [main_file, app_file, constants_file]
        |> List.iter(Assert.file_exists);

        Assert.string(
          "var $knot = require(\"@knot/runtime\");
var $import$_$common$constants = require(\"./common/constants\");
var TIMEOUT = $import$_$common$constants.main;
$import$_$common$constants = null;
var $import$_$App = require(\"./App\");
var App = $import$_$App.main;
$import$_$App = null;
var ABC = (123 / TIMEOUT);
exports.ABC = ABC;
",
          Util.read_file_to_string(main_file),
        );

        Assert.string(
          "var $knot = require(\"@knot/runtime\");
var App = $knot.jsx.createTag(\"div\", null, \"hello world\");
exports.App = App;
exports.main = App;
",
          Util.read_file_to_string(app_file),
        );

        Assert.string(
          "var $knot = require(\"@knot/runtime\");
var TIMEOUT = 100;
exports.TIMEOUT = TIMEOUT;
exports.main = TIMEOUT;
",
          Util.read_file_to_string(constants_file),
        );
      }
    ),
  ];
