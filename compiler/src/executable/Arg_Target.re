open Kore;

let __targets = [
  Target.javascript_es6,
  Target.javascript_common,
  Target.knot,
];

let create = () => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="t",
      ~options=__targets,
      ~from_config=
        cfg => cfg.target |?> ~@Target.pp % (x => Argument.Value.String(x)),
      target_key,
      Symbol(__targets, x => value := Some(target_of_string(x))),
      "the target to compile to",
    );
  let resolve = (~argv=Sys.argv, cfg: option(Config.t)) => {
    let result =
      switch (cfg, value^) {
      | (_, Some(value)) => value
      | (Some({target: Some(target), _}), None) => target
      | (_, _) =>
        InvalidArgument(target_key, "must provide a target for compilation")
        |> fatal
      };
    let ambient =
      result |> Target.to_ambient_lib |> Resource.Asset.find(~argv);

    (result, ambient);
  };

  (argument, resolve);
};
