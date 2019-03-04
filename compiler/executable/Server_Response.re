open Httpaf;

let content_type_header = ("Content-Type", "application/json");
let close_header = ("Connection", "close");

let ok = () =>
  Response.create(
    ~headers=Headers.of_list([content_type_header, close_header]),
    `OK,
  );

let not_allowed = () =>
  Response.create(
    ~headers=Headers.of_list([close_header]),
    `Method_not_allowed,
  );
