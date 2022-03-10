open Kore;
open Fswatch;
open Lwt;

/**
 watches a directory structure and reacts to files changing
 */
type t = {
  dir: string,
  extensions: list(string),
};

type action_t =
  | Add
  | Remove
  | Update
  | Relocate;

type dispatch_t = list((string, action_t)) => unit;

let _ext_matches = (path: string, watcher: t): bool =>
  watcher.extensions |> List.exists(ext => String.ends_with(ext, path));

let _flag_to_string: Fswatch.Event.flag => string =
  Fswatch.Event.(
    fun
    | NoOp => "NoOp"
    | PlatformSpecific => "PlatformSpecific"
    | Created => "Created"
    | Updated => "Updated"
    | Removed => "Removed"
    | Renamed => "Renamed"
    | OwnerModified => "OwnerModified"
    | AttributeModified => "AttributeModified"
    | MovedFrom => "MovedFrom"
    | MovedTo => "MovedTo"
    | IsFile => "IsFile"
    | IsDir => "IsDir"
    | IsSymLink => "IsSymLink"
    | Link => "Link"
    | Overflow => "Overflow"
  );

let rec _listen = (dispatch: dispatch_t, watcher: t, msgBox) =>
  Lwt_mvar.take(msgBox)
  >>= (
    events => {
      events
      |> Array.to_list
      |> List.filter_map(
           Event.(
             event => {
               let path = event.path |> String.drop_prefix(watcher.dir);

               print_endline("file found: " ++ path);
               print_endline(
                 "flags: "
                 ++ (
                   event.flags
                   |> Array.map(_flag_to_string)
                   |> Array.fold_left((++), "")
                 ),
               );

               (
                 switch (event.flags) {
                 /*
                   ignore directories and symlinks
                   using a negation check as some platforms do not use the IsFile flag
                  */
                 | flags
                     when
                       [IsDir, IsSymLink]
                       |> List.exists(flag => Array.mem(flag, flags)) =>
                   None

                 /* ignore files with the wrong extension */
                 | flags when !(watcher |> _ext_matches(event.path)) => None

                 | flags when Array.mem(Created, flags) => Some(Add)

                 | flags when Array.mem(Updated, flags) => Some(Update)

                 | flags when Array.mem(Removed, flags) => Some(Remove)

                 | flags
                     when
                       [Renamed, MovedFrom, MovedTo]
                       |> List.exists(flag => Array.mem(flag, flags)) =>
                   Some(Relocate)

                 /* ignore these flags if they appear on their own */
                 | [|NoOp | Overflow|] => None

                 /* treat everything else as an update */
                 | _ => Some(Update)
                 }
               )
               |> (
                 fun
                 | Some(action) => Some((path, action))
                 | None => None
               );
             }
           ),
         )
      |> dispatch;

      /*
        TODO: might have to change this back to stdout
        the thinking is that all log messages are written to stderr so shouldn't have
        anything to flush on stdout
       */
      flush(stderr);
      _listen(dispatch, watcher, msgBox);
    }
  );

/* static */

let create = (dir: string, extensions: list(string)) => {
  switch (init_library()) {
  | Status.FSW_OK => ()
  | status => raise(WatchFailed(Status.t_to_string(status)))
  };

  {dir, extensions};
};

/* methods */

/**
 start watching a folder for file system events
 */
let watch = (dispatch: dispatch_t, watcher: t) => {
  let (handle, msgBox) = Fswatch_lwt.init_session(Monitor.System_default);

  add_path(handle, watcher.dir);

  Lwt.async(Fswatch_lwt.start_monitor(handle));

  _listen(dispatch, watcher, msgBox);
};
