open Kore;

let start = () => {
  Observer.of_channel(stdin);

  Lwt.return();
};
