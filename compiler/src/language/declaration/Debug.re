open Kore;
open AST;

let to_xml: (Type.t => string, Result.declaration_t) => Fmt.xml_t(string) =
  dump_type => {
    let bind = to_xml => to_xml(dump_type) % List.single;
    let unpack =
      Declaration.fold(
        ~constant=bind(KConstant.to_xml),
        ~enumerated=bind(KEnumerated.to_xml),
        ~function_=bind(KFunction.to_xml),
        ~view=bind(KView.to_xml),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Declaration");
  };
