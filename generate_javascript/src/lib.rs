mod javascript;

// TODO: move these to a common library to be re-used across generators

#[derive(PartialEq)]
pub enum Mode {
    Dev,
    Prod,
}

pub struct Options {
    mode: Mode,
}
