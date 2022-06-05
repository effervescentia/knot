include Knot.Kore;
include Test.Infix;

let __stdlib = Resource.Asset.find("stdlib.kd");

let fixture = Filename.concat("./test/e2e/.fixtures");

let process_build_cmd = (cwd, argv) => {
  let (global, cmd) =
    Executable.Processor.run(~cwd, ~argv, ~color=false, __stdlib);
  let config =
    cmd
    |> (
      fun
      | Build(cfg) => cfg
      | _ => Assert.fail("expected to resolve the build command")
    );

  (global, config);
};
