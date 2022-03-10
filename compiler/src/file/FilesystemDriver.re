open Kore;

type action_t =
  | Add
  | Remove
  | Update
  | Relocate;

module type T = {
  let handle: (string, array(Fswatch.Event.flag)) => option(action_t);
};

module Make = (Driver: T) => {
  include Driver;
};

let _is_update =
  Fswatch.Event.(
    Array.exists(
      fun
      | Updated
      | OwnerModified
      | AttributeModified => true
      | _ => false,
    )
  );

module Darwin = {
  let tree = FileTree.create();

  include Make({
    open Fswatch.Event;

    let handle = (path, flags) => {
      let path' = Filename.split(path);
      let file_exists = tree |> FileTree.exists(path');

      switch (file_exists) {
      /* file was created */
      | false when flags |> Array.mem(Created) =>
        Lwt.async(() => tree |> FileTree.add(path') |> Lwt.return);
        Some(Add);

      /* file was removed */
      | true when flags |> Array.mem(Removed) =>
        Lwt.async(() => tree |> FileTree.remove(path') |> Lwt.return);
        Some(Remove);

      /* file was updated */
      | _ when _is_update(flags) =>
        if (!file_exists) {
          Lwt.async(() => tree |> FileTree.add(path') |> Lwt.return);
        };

        Some(Update);

      | _ => None
      };
    };
  });
};

module Linux = Darwin;

module Windows = Linux;

module Cygwin = Linux;

include Make({
  let handle =
    switch (Platform.get()) {
    | Darwin => Darwin.handle
    | Linux => Linux.handle
    | Windows => Windows.handle
    | Cygwin => Cygwin.handle
    };
});
