open Kore;

let primitive_to_xml =
  Interface.fold(
    ~nil=(@@)(KNil.to_xml),
    ~boolean=(@@)(KBoolean.to_xml),
    ~integer=(@@)(KInteger.to_xml),
    ~float=(@@)(KFloat.to_xml),
    ~string=(@@)(KString.to_xml),
  );

let to_xml: Interface.Plugin.debug_t('expr, 'typ) = _ => primitive_to_xml;
