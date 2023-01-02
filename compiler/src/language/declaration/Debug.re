open Kore;
open AST;

let to_xml: (Type.t => string, Module.declaration_t) => Fmt.xml_t(string) =
  dump_type => {
    let (&>) = (args, to_xml) => args |> to_xml(dump_type) |> List.single;

    Dump.node_to_xml(
      ~dump_type,
      ~unpack=
        Module.(
          fun
          | Constant(expr) => expr &> KConstant.to_xml
          | Enumerated(variants) => variants &> KEnumerated.to_xml
          | Function(parameters, body) =>
            (parameters, body) &> KFunction.to_xml
          | View(parameters, mixins, body) =>
            (parameters, mixins, body) &> KView.to_xml
        ),
      "Declaration",
    );
  };
