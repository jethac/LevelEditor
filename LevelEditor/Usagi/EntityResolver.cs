using System;
using System.ComponentModel.Composition;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using Sce.Atf;
using LevelEditorCore;
using System.IO;

namespace Vitei.RenderingInteropUsagi
{
    [Export(typeof (IResourceResolver))]
    [PartCreationPolicy(CreationPolicy.Shared)]
    public class EntityResolver : IResourceResolver
    {
        public const string ResourceType = "UsagiEntity";

        IResource IResourceResolver.Resolve(Uri uri)
        {
            IResource resource = null;
            string fileName = uri.LocalPath;
            string ext = Path.GetExtension(fileName).ToLower();
            var res = m_gameEngine.Info.ResourceInfos.GetByType(ResourceType);
            if (res.IsSupported(ext))
                resource = new EntityResource(uri, ResourceType);

            return resource;
        }

        private class EntityResource : IResource
        {
            public EntityResource(Uri uri, string type)
            {
                m_uri = uri;
                m_type = type;
            }

            public string Type
            {
                get { return m_type; }
            }
            public Uri Uri
            {
                get { return m_uri; }
                set
                {
                    throw new InvalidOperationException();
                }
            }

            public event EventHandler<UriChangedEventArgs> UriChanged
                = delegate { };

            private string m_type;
            private Uri m_uri;
        }

        [Import(AllowDefault = false)]
        private IGameEngineProxy m_gameEngine;
    }
}
