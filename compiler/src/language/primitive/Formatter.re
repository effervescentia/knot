open Kore;

let format: Interface.Plugin.format_t('expr, 'typ) =
  (_, _, ppf) => {
    let bind = format => format(ppf);

    Interface.fold(
      ~nil=bind(KNil.format),
      ~boolean=bind(KBoolean.format),
      ~integer=bind(KInteger.format),
      ~float=bind(KFloat.format),
      ~string=bind(KString.format),
    );
  };
