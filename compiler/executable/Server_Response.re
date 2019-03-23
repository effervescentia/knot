open Httpaf;

let content_type_header = ("Content-Type", "text/plain;charset=utf-8");
let close_header = ("Connection", "close");
let std_headers = Headers.of_list([content_type_header, close_header]);

let ok = () => Response.create(~headers=std_headers, `OK);

let not_allowed = () =>
  Response.create(~headers=std_headers, `Method_not_allowed);

let not_found = () => Response.create(~headers=std_headers, `Not_found);
