open Knot.Kore;

let parse = Parser.declaration;

let pp = Formatter.pp_declaration;

let to_xml:
  (AST.Type.t => string, (AST.Module.export_t, AST.Module.declaration_t)) =>
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
            AST.Module.(
              fun
              | Constant(expr) => KConstant.Plugin.to_xml(dump_type, expr)
              | Enumerated(variants) =>
                KEnumerated.Plugin.to_xml(dump_type, variants)
              | Function(parameters, body) =>
                KFunction.Plugin.to_xml(
                  KExpression.Plugin.to_xml(dump_type),
                  dump_type,
                  (parameters, body),
                )
              | View(parameters, mixins, body) =>
                KView.Plugin.to_xml(
                  KExpression.Plugin.to_xml(dump_type),
                  dump_type,
                  (parameters, body),
                )
            )
            % (x => [x]),
          "Entity",
          decl,
        ),
      ],
    );
