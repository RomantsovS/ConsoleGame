#ifndef ACTOR_H
#define ACTOR_H

class idAttachInfo {
 public:
  std::shared_ptr<idEntity> ent;
};

class idActor : public idAnimatedEntity {
 public:
  CLASS_PROTOTYPE(idActor);

  idActor();
  virtual ~idActor() override;
  idActor(const idActor&) = default;
  idActor& operator=(const idActor&) = default;
  idActor(idActor&&) = default;
  idActor& operator=(idActor&&) = default;

  void Spawn();

  void Attach(std::shared_ptr<idEntity> ent);

  int GetAnim(const std::string& name);

 protected:
  std::list<idAttachInfo> attachments;
};

#endif