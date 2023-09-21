use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Visibility {
    Public,
    Private,
}

#[derive(Clone, Debug, PartialEq)]
pub struct Storage(pub Visibility, pub String);
