include Knot.Kore;
include Test.Kore;

let fixture_dir = "./test/file/.fixtures";

let _fixture = Filename.concat(fixture_dir);

let fixture_path = _fixture("read_me.txt");
let multiline_path = _fixture("multiline.txt");
