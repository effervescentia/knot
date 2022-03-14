open Kore;

type action_t =
  | Add
  | Remove
  | Update;

type t = (string, array(Fswatch.Event.flag)) => option(action_t);

module type T = {
  let create: (unit, string, array(Fswatch.Event.flag)) => option(action_t);
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

let _is_add =
  Fswatch.Event.(
    Array.exists(
      fun
      | MovedTo
      | Renamed
      | Created => true
      | _ => false,
    )
  );

let _is_remove =
  Fswatch.Event.(
    Array.exists(
      fun
      | MovedFrom
      | Removed => true
      | _ => false,
    )
  );

module Darwin =
  Make({
    open Fswatch.Event;

    let create = () => {
      let tree = FileTree.create();

      (path, flags) => {
        let path' = Filename.split(path);
        let file_exists = tree |> FileTree.exists(path');

        switch (file_exists) {
        /* file was removed */
        | _ when _is_remove(flags) =>
          tree |> FileTree.remove(path');
          Some(Remove);

        | true when flags |> Array.mem(Renamed) =>
          tree |> FileTree.remove(path');
          Some(Remove);

        /* file was added */
        | false when _is_add(flags) =>
          tree |> FileTree.add(path');
          Some(Add);

        /* file was updated */
        | _ when _is_update(flags) =>
          if (!file_exists) {
            tree |> FileTree.add(path');
          };

          Some(Update);

        | _ => None
        };
      };
    };
  });

module Linux = Darwin;

module Windows = Linux;

module Cygwin = Linux;

include Make({
  let create =
    switch (Platform.get()) {
    | Darwin => Darwin.create
    | Linux => Linux.create
    | Windows => Windows.create
    | Cygwin => Cygwin.create
    };
});
