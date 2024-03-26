pub use lang::ast::*;

pub mod typed {
    pub use super::ctx::Binding;
    use lang::types;
    use std::rc::Rc;

    #[derive(Clone, Debug, PartialEq)]
    pub struct Type(pub types::Type<Rc<Type>>);

    pub type Storage = super::Storage<Binding>;
    pub type Expression = super::ctx::Expression<Type>;
    pub type Statement = super::ctx::Statement<Type>;
    pub type Component = super::ctx::Component<Type>;
    pub type TypeExpression = super::ctx::TypeExpression<Type>;
    pub type Parameter = super::ctx::Parameter<Type>;
    pub type Declaration = super::ctx::Declaration<Type>;
    pub type Import = super::ctx::Import<Type>;
    pub type Module = super::ctx::Module<Type>;

    #[derive(Debug, PartialEq)]
    pub struct Program(pub Module);
}
