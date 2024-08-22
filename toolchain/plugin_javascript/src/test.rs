use crate::{resolve::ImportResolver, Mode, Module, Options};
use std::{fmt::Display, str::FromStr};

#[derive(Default)]
pub struct MockLibrary;

impl FromStr for MockLibrary {
    type Err = ();

    fn from_str(_: &str) -> Result<Self, Self::Err> {
        Err(())
    }
}

impl Display for MockLibrary {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str("<library>")
    }
}

pub const MOCK_OPTIONS: Options<MockLibrary> = Options {
    mode: Mode::Production,
    resolver: ImportResolver::new(Module::ESM),
};
