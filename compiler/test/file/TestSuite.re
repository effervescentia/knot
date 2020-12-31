open Kore;

let () = {
  "File" >:::: [CacheTest.suite, ReaderTest.suite, WriterTest.suite];
};
