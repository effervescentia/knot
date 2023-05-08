open Kore;
open AST;

let to_xml: ('typ => string, Interface.node_t('typ)) => Fmt.xml_t(string) =
  dump_type => {
    let bind = to_xml => to_xml(dump_type) % List.single;
    let unpack =
      Interface.fold(
        ~constant=bind(Constant.to_xml),
        ~enumerated=bind(Enumerated.to_xml),
        ~function_=bind(Function.to_xml),
        ~view=bind(View.to_xml),
      );

    Dump.node_to_xml(~dump_type, ~unpack, "Declaration");
  };
