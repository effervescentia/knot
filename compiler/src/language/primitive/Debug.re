open Kore;

let to_xml: Interface.Plugin.debug_t('expr, 'typ) =
  _ =>
    Interface.fold(
      ~nil=(@@)(KNil.to_xml),
      ~boolean=(@@)(KBoolean.to_xml),
      ~integer=(@@)(KInteger.to_xml),
      ~float=(@@)(KFloat.to_xml),
      ~string=(@@)(KString.to_xml),
    );
