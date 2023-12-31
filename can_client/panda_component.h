/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include <memory>

#include "cyber/component/component.h"
#include "panda/panda.h"

using apollo::cyber::Component;
using apollo::cyber::ComponentBase;

namespace commaai {
class PandaComponent : public Component<Panda> {
public:
    bool Init() override;

    bool Proc(const std::shared_ptr<Panda> &panda) override;
};
}

CYBER_REGISTER_COMPONENT(commaai::PandaComponent)
