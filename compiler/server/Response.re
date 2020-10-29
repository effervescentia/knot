open Httpaf;

let _content_type_header = ("Content-Type", "text/plain;charset=utf-8");
let _close_header = ("Connection", "close");
let _std_headers = Headers.of_list([_content_type_header, _close_header]);

let ok = () => Response.create(~headers=_std_headers, `OK);

let not_allowed = () =>
  Response.create(~headers=_std_headers, `Method_not_allowed);

let not_found = () => Response.create(~headers=_std_headers, `Not_found);
