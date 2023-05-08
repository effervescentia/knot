open Kore;

let create = () => {
  let value = ref(false);
  let argument =
    Argument.create(
      "help",
      Unit(() => value := true),
      "display this list of options",
    );
  let hidden_argument = ("-help", Arg.Unit(() => value := true), "");
  let resolve = () => value^;

  ((argument, hidden_argument), resolve);
};
