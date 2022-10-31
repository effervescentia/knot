module XML = {
  open Pretty.XML;

  let few_attributes = [("fizz", "buzz"), ("zip", "zap")];

  let many_attributes = [
    ("Ut", "ex"),
    ("ex", "veniam"),
    ("nostrud", "dolor"),
    ("tempor", "mollit"),
    ("id", "commodo"),
    ("deserunt", "pariatur"),
    ("ipsum", "pariatur"),
  ];

  let with_few_attributes = Node("foo", few_attributes, []);

  let with_many_attributes = Node("foo", many_attributes, []);
};

module List = {
  let abc = ["a", "b", "c"];

  let def = ["d", "e", "f"];

  let a1b2c3 = [("a", 1), ("b", 2), ("c", 3)];

  let d4e5f6 = [("d", 4), ("e", 5), ("f", 6)];

  let many = [
    "Sint",
    "eiusmod",
    "quis",
    "consectetur",
    "cillum",
    "nulla",
    "est",
    "et",
    "ipsum",
    "nisi",
  ];

  let too_many = ["Lorem", "ea", ...many];
};
