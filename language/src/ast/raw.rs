pub use super::meta::Binding;

pub type Storage = super::Storage<Binding>;
pub type Expression = super::meta::Expression<()>;
pub type Statement = super::meta::Statement<()>;
pub type Component = super::meta::Component<()>;
pub type TypeExpression = super::meta::TypeExpression<()>;
pub type Parameter = super::meta::Parameter<()>;
pub type Declaration = super::meta::Declaration<()>;
pub type Import = super::meta::Import<()>;
pub type Module = super::meta::Module<()>;
pub type Program = super::meta::Program<()>;

pub type TypeDeclaration = super::meta::TypeDeclaration<()>;
pub type TypeModule = super::meta::TypeModule<()>;
pub type Typings = super::meta::Typings<()>;
