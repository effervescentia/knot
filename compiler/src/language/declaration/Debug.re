open Kore;
open AST;

let entity_to_xml = dump_type =>
  Dump.node_to_xml(
    ~dump_type,
    ~unpack=
      Module.(
        fun
        | Constant(expr) => KConstant.to_xml(dump_type, expr)
        | Enumerated(variants) => KEnumerated.to_xml(dump_type, variants)
        | Function(parameters, body) =>
          KFunction.to_xml(dump_type, (parameters, body))
        | View(parameters, mixins, body) =>
          KView.to_xml(dump_type, (parameters, mixins, body))
      )
      % (x => [x]),
    "Entity",
  );

let to_xml:
  (
    Type.t => string,
    (Module.export_t, Common.identifier_t, Module.declaration_t)
  ) =>
  Fmt.xml_t(string) =
  (dump_type, (export, name, decl)) =>
    Node(
      "Declaration",
      [],
      [
        Dump.node_to_xml(
          ~dump_value=Fun.id,
          switch (export) {
          | Named => "Named"
          | Main => "Main"
          },
          name,
        ),
        entity_to_xml(dump_type, decl),
      ],
    );
