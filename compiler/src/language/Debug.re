open Kore;

let program_to_xml:
  ('typ => string, Interface.program_t('typ)) => Fmt.xml_t(string) =
  (dump_type, program) =>
    Node(
      "Program",
      [],
      program
      |> List.map(ModuleStatement.to_xml(Declaration.to_xml(dump_type))),
    );
