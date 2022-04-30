open Kore;

type message_t =
  | Error
  | Warning
  | Info
  | Log;

let method_key = "window/showMessage";

let notification = (message: string, type_: message_t) =>
  `Assoc([
    ("message", `String(message)),
    (
      "type",
      `Int(
        switch (type_) {
        | Error => 1
        | Warning => 2
        | Info => 3
        | Log => 4
        },
      ),
    ),
  ]);

let send = ({server}: Runtime.t, message: string, type_: message_t) =>
  notification(message, type_) |> server.notify(method_key);
