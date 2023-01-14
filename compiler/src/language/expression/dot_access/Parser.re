open Knot.Kore;
open Parse.Kore;

let parse: Interface.Plugin.parse_t('ast, 'expr) =
  (f, parse_expression) => {
    let rec loop = object_ =>
      Matchers.period
      >> Matchers.identifier
      >>= (
        property =>
          Node.raw(
            (object_, property) |> f,
            Node.join_ranges(object_, property),
          )
          |> loop
      )
      |> option(object_);

    parse_expression >>= loop;
  };
