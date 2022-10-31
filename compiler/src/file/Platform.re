type t =
  | Linux
  | Darwin
  | Windows
  | Cygwin;

let get = (): t =>
  switch (Sys.os_type) {
  | "Win32" => Windows
  | "Cygwin" => Cygwin
  | _ =>
    switch (Util.uname()) {
    | "Darwin" => Darwin
    | _ => Linux
    }
  };

let is_ = (expected: t, ()): bool => get() == expected;

let is_darwin = is_(Darwin);
