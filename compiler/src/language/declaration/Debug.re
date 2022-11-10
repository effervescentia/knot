open Knot.Kore;
open AST;

let to_xml:
  (Type.t => string, (Module.export_t, Module.declaration_t)) =>
  Fmt.xml_t(string) =
  (dump_type, (name, decl)) =>
    Node(
      "Declaration",
      [],
      [
        switch (name) {
        | MainExport(name') =>
          Dump.node_to_xml(~dump_value=Fun.id, "MainExport", name')
        | NamedExport(name') =>
          Dump.node_to_xml(~dump_value=Fun.id, "NamedExport", name')
        },
        Dump.node_to_xml(
          ~dump_type,
          ~unpack=
            Module.(
              fun
              | Constant(expr) => KConstant.Plugin.to_xml(dump_type, expr)
              | Enumerated(variants) =>
                KEnumerated.Plugin.to_xml(dump_type, variants)
              | Function(parameters, body) =>
                KFunction.Plugin.to_xml(dump_type, (parameters, body))
              | View(parameters, mixins, body) =>
                KView.Plugin.to_xml(dump_type, (parameters, mixins, body))
            )
            % (x => [x]),
          "Entity",
          decl,
        ),
      ],
    );
