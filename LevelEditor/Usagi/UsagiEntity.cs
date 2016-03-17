using System.IO;

using LevelEditorCore;

using Sce.Atf;
using Sce.Atf.Adaptation;
using Sce.Atf.Applications;
using Sce.Atf.Dom;
using LevelEditor.DomNodeAdapters;

namespace LevelEditor.Usagi
{
    public class UsagiEntity : GameObject
    {
        /// <summary>
        /// Gets or sets the resource reference</summary>
        public IReference<IResource> Reference
        {
            get { return GetChild<IReference<IResource>>(Schema.usagiEntityType.resourceChild); }
            set { SetChild(Schema.usagiEntityType.resourceChild, value.As<DomNode>()); }
        }
        public static UsagiEntity Create()
        {
            UsagiEntity entity = new DomNode(Schema.usagiEntityType.Type).As<UsagiEntity>();
            entity.Name = "Entity".Localize();
            return entity;
        }
    }
}
