open Core;

let is_resolved = ((_, promise)) =>
  switch (promise^ ^) {
  | Resolved(_) => true
  | _ => false
  };
