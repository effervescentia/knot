open Kore;
open Yojson.Basic.Util;

module Response = LanguageServer.Response;

let suite =
  "LanguageServer.Response"
  >::: [
    "serialize() - empty"
    >: (
      () =>
        Assert.string(
          "Content-Length: 2\r\n\r\n{}",
          Response.serialize(`Assoc([])),
        )
    ),
  ];
