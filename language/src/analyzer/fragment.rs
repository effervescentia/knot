use super::{infer::weak::ToWeak, WeakRef};
use crate::parser::{
    declaration::{storage::Storage, Declaration},
    expression::{ksx::KSX, statement::Statement, Expression},
    module::Module,
    types::type_expression::TypeExpression,
};
use std::fmt::Debug;

#[derive(Debug, PartialEq)]
pub enum Fragment {
    Expression(Expression<usize, usize, usize>),
    Statement(Statement<usize>),
    KSX(KSX<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize>),
    Module(Module<usize>),
}

impl Fragment {
    pub fn to_binding(&self) -> Option<String> {
        match self {
            Fragment::Statement(Statement::Variable(name, ..))
            | Fragment::Declaration(
                Declaration::TypeAlias {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Enumerated {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Constant {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Function {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::View {
                    name: Storage(_, name),
                    ..
                }
                | Declaration::Module {
                    name: Storage(_, name),
                    ..
                },
            ) => Some(name.clone()),

            _ => None,
        }
    }
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> WeakRef {
        match self {
            Fragment::Expression(x) => x.to_weak(),
            Fragment::Statement(x) => x.to_weak(),
            Fragment::KSX(x) => x.to_weak(),
            Fragment::TypeExpression(x) => x.to_weak(),
            Fragment::Declaration(x) => x.to_weak(),
            Fragment::Module(x) => x.to_weak(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use crate::{parser::expression::statement::Statement, test::fixture as f};

    #[test]
    fn binding_variable() {
        assert_eq!(
            Fragment::Statement(Statement::Variable(String::from("foo"), 0)).to_binding(),
            Some(String::from("foo"))
        );
    }

    #[test]
    fn binding_type_alias() {
        assert_eq!(
            Fragment::Declaration(f::a::type_("Foo", 0)).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_enumerated() {
        assert_eq!(
            Fragment::Declaration(f::a::enum_("Foo", vec![])).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_constant() {
        assert_eq!(
            Fragment::Declaration(f::a::const_("FOO", None, 0)).to_binding(),
            Some(String::from("FOO"))
        );
    }

    #[test]
    fn binding_function() {
        assert_eq!(
            Fragment::Declaration(f::a::func_("foo", vec![], None, 0)).to_binding(),
            Some(String::from("foo"))
        );
    }

    #[test]
    fn binding_view() {
        assert_eq!(
            Fragment::Declaration(f::a::view("Foo", vec![], 0)).to_binding(),
            Some(String::from("Foo"))
        );
    }

    #[test]
    fn binding_module() {
        assert_eq!(
            Fragment::Declaration(f::a::mod_("foo", 0)).to_binding(),
            Some(String::from("foo"))
        );
    }
}
