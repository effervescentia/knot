use super::infer::weak::{ToWeak, WeakRef};
use lang::ast::{
    Declaration, Expression, Import, Module, Parameter, Statement, TypeExpression, KSX,
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
#[allow(clippy::upper_case_acronyms)]
pub enum Fragment {
    Expression(Expression<usize, usize, usize>),
    Statement(Statement<usize>),
    KSX(KSX<usize, usize>),
    Parameter(Parameter<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize, usize>),
    Import(Import),
    Module(Module<usize, usize>),
}

impl Fragment {
    pub fn to_binding(&self) -> Result<Vec<String>, ()> {
        match self {
            Self::Statement(Statement::Variable(name, ..))
            | Self::Parameter(Parameter { name, .. }) => Ok(vec![name.clone()]),

            Self::Declaration(x) => Ok(vec![x.name().clone()]),

            Self::Import(Import {
                path, alias: None, ..
            }) => Ok(vec![path.last().ok_or(())?.clone()]),

            Self::Import(Import {
                alias: Some(alias), ..
            }) => Ok(vec![alias.clone()]),

            _ => Ok(vec![]),
        }
    }
}

impl ToWeak for Fragment {
    fn to_weak(&self) -> WeakRef {
        match self {
            Self::Expression(x) => x.to_weak(),
            Self::Statement(x) => x.to_weak(),
            Self::KSX(x) => x.to_weak(),
            Self::Parameter(x) => x.to_weak(),
            Self::TypeExpression(x) => x.to_weak(),
            Self::Declaration(x) => x.to_weak(),
            Self::Import(x) => x.to_weak(),
            Self::Module(x) => x.to_weak(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::Fragment;
    use kore::{assert_eq, str};
    use lang::{
        ast::{Import, ImportSource, Statement},
        test::fixture as f,
    };

    #[test]
    fn binding_variable() {
        assert_eq!(
            Fragment::Statement(Statement::Variable(str!("foo"), 0))
                .to_binding()
                .unwrap(),
            vec![str!("foo")]
        );
    }

    #[test]
    fn binding_type_alias() {
        assert_eq!(
            Fragment::Declaration(f::a::type_("Foo", 0))
                .to_binding()
                .unwrap(),
            vec![str!("Foo")]
        );
    }

    #[test]
    fn binding_enumerated() {
        assert_eq!(
            Fragment::Declaration(f::a::enum_("Foo", vec![]))
                .to_binding()
                .unwrap(),
            vec![str!("Foo")]
        );
    }

    #[test]
    fn binding_constant() {
        assert_eq!(
            Fragment::Declaration(f::a::const_("FOO", None, 0))
                .to_binding()
                .unwrap(),
            vec![str!("FOO")]
        );
    }

    #[test]
    fn binding_function() {
        assert_eq!(
            Fragment::Declaration(f::a::func_("foo", vec![], None, 0))
                .to_binding()
                .unwrap(),
            vec![str!("foo")]
        );
    }

    #[test]
    fn binding_view() {
        assert_eq!(
            Fragment::Declaration(f::a::view("Foo", vec![], 0))
                .to_binding()
                .unwrap(),
            vec![str!("Foo")]
        );
    }

    #[test]
    fn binding_module_import() {
        assert_eq!(
            Fragment::Import(Import {
                source: ImportSource::Local,
                path: vec![str!("foo"), str!("bar")],
                alias: None
            })
            .to_binding()
            .unwrap(),
            vec![str!("bar")]
        );
    }

    #[test]
    fn binding_module_import_with_alias() {
        assert_eq!(
            Fragment::Import(Import {
                source: ImportSource::Local,
                path: vec![str!("foo"), str!("bar")],
                alias: Some(str!("fizz"))
            })
            .to_binding()
            .unwrap(),
            vec![str!("fizz")]
        );
    }

    #[test]
    fn binding_module() {
        assert_eq!(
            Fragment::Declaration(f::a::module("foo", 0))
                .to_binding()
                .unwrap(),
            vec![str!("foo")]
        );
    }
}
