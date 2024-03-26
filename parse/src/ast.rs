pub use lang::ast::*;

pub mod raw {
    pub use super::ctx::Binding;
    use super::walk::Walk;

    pub type Storage = super::Storage<Binding>;
    pub type Expression = super::ctx::Expression<()>;
    pub type Statement = super::ctx::Statement<()>;
    pub type Component = super::ctx::Component<()>;
    pub type TypeExpression = super::ctx::TypeExpression<()>;
    pub type Parameter = super::ctx::Parameter<()>;
    pub type Declaration = super::ctx::Declaration<()>;
    pub type Import = super::ctx::Import<()>;
    pub type Module = super::ctx::Module<()>;

    pub struct Program(pub Module);

    impl Program {
        pub fn to_shape(self) -> super::shape::Module {
            self.0.walk(super::shape::Visitor).0
        }
    }
}
