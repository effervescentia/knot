open Kore;

let suite =
  "Library.Fun"
  >::: [
    "generator()"
    >: (
      () => {
        let next = Fun.generator([1, 2, 3]);

        Assert.opt_int(Some(1), next());
        Assert.opt_int(Some(2), next());
        Assert.opt_int(Some(3), next());
        Assert.opt_int(None, next());
      }
    ),
  ];
