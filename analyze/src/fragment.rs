use super::infer::weak::{ToWeak, WeakRef};
use lang::ast::{Declaration, Expression, Module, Parameter, Statement, TypeExpression, KSX};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Fragment {
    Expression(Expression<usize, usize, usize>),
    Statement(Statement<usize>),
    KSX(KSX<usize, usize>),
    Parameter(Parameter<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize, usize>),
    Module(Module<usize>),
}

impl Fragment {
    pub fn to_binding(&self) -> Option<String> {
        match self {
            Fragment::Statement(Statement::Variable(name, ..))
            | Fragment::Parameter(Parameter { name, .. }) => Some(name.clone()),

            Fragment::Declaration(x) => Some(x.name().clone()),

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
            Fragment::Parameter(x) => x.to_weak(),
            Fragment::TypeExpression(x) => x.to_weak(),
            Fragment::Declaration(x) => x.to_weak(),
            Fragment::Module(x) => x.to_weak(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use lang::{ast::Statement, test::fixture as f};

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
            Fragment::Declaration(f::a::module("foo", 0)).to_binding(),
            Some(String::from("foo"))
        );
    }
}
