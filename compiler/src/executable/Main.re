open Kore;

type mode =
  | Static
  | Server;

let run = () => {
  let mode = Static;
  let name = "";
  let entry = Internal("");
  let root_dir = "";

  switch (mode) {
  | Static => Static.run({name, root_dir, entry})
  | Server =>
    Server.run({name, root_dir, port: 8080, watch: Some({entry: entry})})
  };
};
