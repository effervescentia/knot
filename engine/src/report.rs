use crate::Link;

#[derive(Clone, Eq, Debug, PartialEq)]
pub enum Error {
    ImportCycle(Vec<Link>),
}
